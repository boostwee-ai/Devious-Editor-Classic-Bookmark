#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

#include "Session.hpp"
#include "../utils/Platform.hpp"
#include <Geode/Geode.hpp>

#ifndef _WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

using namespace geode::prelude;

namespace network {

Session& Session::get() {
    static Session instance;
    return instance;
}

Session::~Session() {
    disconnect();
}

bool Session::startHost() {
    if (m_state.load() != SessionState::Disconnected) return false;

    if (!dutils::Platform::initializeSockets()) return false;

    m_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSock < 0) return false;

    int opt = 1;
    setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(41821);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        dutils::Platform::closeSocket(m_listenSock);
        m_listenSock = -1;
        return false;
    }

    if (listen(m_listenSock, 1) == -1) {
        dutils::Platform::closeSocket(m_listenSock);
        m_listenSock = -1;
        return false;
    }

    m_state.store(SessionState::HostListening);
    m_acceptThread = std::thread(&Session::hostAcceptLoop, this);
    return true;
}

bool Session::connectToPeer(const std::string& ip) {
    if (m_state.load() != SessionState::Disconnected) return false;

    if (!dutils::Platform::initializeSockets()) return false;

    m_peerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_peerSock < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(41821);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(m_peerSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        dutils::Platform::closeSocket(m_peerSock);
        m_peerSock = -1;
        return false;
    }

    m_peerIp = ip;
    m_state.store(SessionState::Connected);
    
    m_receiveThread = std::thread(&Session::receiveLoop, this);
    m_sendThread = std::thread(&Session::flushSendQueue, this);
    return true;
}

void Session::disconnect() {
    SessionState currentState = m_state.exchange(SessionState::Disconnected);
    if (currentState == SessionState::Disconnected) return;

    if (m_listenSock != -1) {
        dutils::Platform::closeSocket(m_listenSock);
        m_listenSock = -1;
    }

    if (m_peerSock != -1) {
        dutils::Platform::closeSocket(m_peerSock);
        m_peerSock = -1;
    }

    if (m_acceptThread.joinable()) m_acceptThread.join();
    if (m_receiveThread.joinable()) m_receiveThread.detach(); // Detach to avoid deadlock if called from receive
    if (m_sendThread.joinable()) m_sendThread.join();

    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        m_sendQueue.clear();
    }
    m_hasDataToSend.store(false);

    if (m_disconnectCallback) {
        geode::Loader::get()->queueInMainThread([cb = m_disconnectCallback] {
            cb();
        });
    }
}

void Session::sendPacket(const std::vector<uint8_t>& packet) {
    if (m_state.load() != SessionState::Connected) return;

    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendQueue.push_back(packet);
    m_hasDataToSend.store(true);
}

SessionState Session::getState() const {
    return m_state.load();
}

std::string Session::getConnectedIP() const {
    if (m_state.load() == SessionState::Connected) return m_peerIp;
    return "";
}

void Session::setPacketCallback(PacketCallback callback) {
    m_packetCallback = callback;
}

void Session::setDisconnectCallback(DisconnectCallback callback) {
    m_disconnectCallback = callback;
}

void Session::hostAcceptLoop() {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientSock = accept(m_listenSock, (struct sockaddr*)&clientAddr, &clientLen);
    
    if (clientSock >= 0) {
        dutils::Platform::closeSocket(m_listenSock);
        m_listenSock = -1;

        m_peerSock = clientSock;
        m_peerIp = inet_ntoa(clientAddr.sin_addr);
        m_state.store(SessionState::Connected);

        m_receiveThread = std::thread(&Session::receiveLoop, this);
        m_sendThread = std::thread(&Session::flushSendQueue, this);
    } else {
        disconnect();
    }
}

void Session::receiveLoop() {
    std::vector<uint8_t> buffer;
    char readBuf[4096];

    while (m_state.load() == SessionState::Connected) {
        int bytes = recv(m_peerSock, readBuf, sizeof(readBuf), 0);
        
        if (bytes <= 0) {
            disconnect();
            break;
        }

        buffer.insert(buffer.end(), readBuf, readBuf + bytes);

        PacketHeader header;
        std::vector<uint8_t> payload;
        while (Protocol::parsePacket(buffer, header, payload)) {
            if (m_packetCallback) {
                // Execute callback on main thread for safety with GD calls
                geode::Loader::get()->queueInMainThread([this, header, payload]() {
                    if (m_packetCallback && m_state.load() == SessionState::Connected) {
                        m_packetCallback(header, payload);
                    }
                });
            }
        }
    }
}

void Session::flushSendQueue() {
    while (m_state.load() == SessionState::Connected) {
        if (m_hasDataToSend.load()) {
            std::vector<std::vector<uint8_t>> toSend;
            {
                std::lock_guard<std::mutex> lock(m_sendMutex);
                std::swap(toSend, m_sendQueue);
                m_hasDataToSend.store(false);
            }

            for (const auto& packet : toSend) {
                int totalSent = 0;
                while (totalSent < packet.size() && m_state.load() == SessionState::Connected) {
                    int sent = send(m_peerSock, (const char*)packet.data() + totalSent, packet.size() - totalSent, 0);
                    if (sent <= 0) {
                        disconnect();
                        break;
                    }
                    totalSent += sent;
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

} // namespace network
