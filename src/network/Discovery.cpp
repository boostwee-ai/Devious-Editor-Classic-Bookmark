#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

#include "Discovery.hpp"
#include "Protocol.hpp"
#include "../utils/Platform.hpp"
#include <Geode/Geode.hpp>
#include <chrono>

#ifndef _WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

using namespace geode::prelude;

namespace network {

Discovery& Discovery::get() {
    static Discovery instance;
    return instance;
}

Discovery::~Discovery() {
    stop();
}

bool Discovery::start() {
    if (m_running.load()) return true;

    if (!dutils::Platform::initializeSockets()) {
        log::error("Failed to initialize sockets");
        return false;
    }

    m_running.store(true);
    
    // Create socket for listening first
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        log::error("Failed to create listening socket");
        return false;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(41820);
    listenAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        log::error("Failed to bind UDP listening socket");
        dutils::Platform::closeSocket(sock);
        return false;
    }

    m_listenSock = sock;

    m_broadcastThread = std::thread(&Discovery::broadcastLoop, this);
    m_listenThread = std::thread(&Discovery::listenLoop, this);

    return true;
}

void Discovery::stop() {
    if (!m_running.load()) return;
    
    m_running.store(false);
    
    if (m_listenSock != -1) {
        dutils::Platform::closeSocket(m_listenSock);
        m_listenSock = -1;
    }

    if (m_broadcastThread.joinable()) m_broadcastThread.join();
    if (m_listenThread.joinable()) m_listenThread.join();
    
    dutils::Platform::cleanupSockets();
}

void Discovery::setLocalState(const std::string& username, bool inEditor, bool collabEnabled) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_username = username;
    m_inEditor = inEditor;
    m_collabEnabled = collabEnabled;
}

std::vector<DiscoveredPeer> Discovery::getActivePeers() {
    std::lock_guard<std::mutex> lock(m_peersMutex);
    std::vector<DiscoveredPeer> active;
    
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (auto it = m_peers.begin(); it != m_peers.end(); ) {
        if (now - it->second.lastSeen > 5000) { // 5 second timeout
            it = m_peers.erase(it);
        } else {
            active.push_back(it->second);
            ++it;
        }
    }
    return active;
}

void Discovery::broadcastLoop() {
    int broadcastSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSock < 0) return;

    int broadcastEnable = 1;
    setsockopt(broadcastSock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcastEnable, sizeof(broadcastEnable));

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(41820);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    while (m_running.load()) {
        std::string username;
        bool inEditor, collabEnabled;
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            username = m_username;
            inEditor = m_inEditor;
            collabEnabled = m_collabEnabled;
        }

        if (!username.empty() && collabEnabled) {
            matjson::Value payload;
            payload["username"] = username;
            payload["inEditor"] = inEditor;
            payload["collabEnabled"] = collabEnabled;
            
            std::string payloadStr = payload.dump(matjson::NO_INDENTATION);
            auto packet = Protocol::createPacket(PacketType::Heartbeat, payloadStr);

            sendto(broadcastSock, (const char*)packet.data(), packet.size(), 0,
                   (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    dutils::Platform::closeSocket(broadcastSock);
}

void Discovery::listenLoop() {
    char buffer[1024];
    sockaddr_in senderAddr{};
    socklen_t senderLen = sizeof(senderAddr);

    while (m_running.load()) {
        int bytesReceived = recvfrom(m_listenSock, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&senderAddr, &senderLen);
        if (bytesReceived <= 0) {
            continue; // Can optionally check for socket close
        }

        std::vector<uint8_t> data(buffer, buffer + bytesReceived);
        PacketHeader header;
        std::vector<uint8_t> payload;

        if (Protocol::parsePacket(data, header, payload)) {
            if (header.type == PacketType::Heartbeat) {
                std::string payloadStr(payload.begin(), payload.end());
                std::string ip = inet_ntoa(senderAddr.sin_addr);

                auto jsonRes = matjson::parse(payloadStr);
                if (jsonRes.isErr()) continue;
                auto json = jsonRes.unwrap();

                if (json.contains("username") && json["username"].isString()) {
                    DiscoveredPeer peer;
                    peer.ip = ip;
                    peer.username = json["username"].asString().unwrapOr("Unknown");
                    peer.inEditor = json["inEditor"].asBool().unwrapOr(false);
                    peer.collabEnabled = json["collabEnabled"].asBool().unwrapOr(false);
                    
                    peer.lastSeen = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                    std::lock_guard<std::mutex> lock(m_peersMutex);
                    // Don't add ourselves (if IP is local interface, tricky to check exactly, 
                    // but we can just filter by username if username == my username, though IP is better)
                    // For now, simplify by checking username
                    std::string myUser;
                    {
                        std::lock_guard<std::mutex> uLock(m_stateMutex);
                        myUser = m_username;
                    }
                    if (myUser != peer.username) {
                        m_peers[ip] = peer;
                    }
                }
            }
        }
    }
}

} // namespace network
