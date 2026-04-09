#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef _WINSOCKAPI_
        #define _WINSOCKAPI_
    #endif
#endif

#include "Protocol.hpp"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace network {

enum class SessionState {
    Disconnected,
    HostListening,
    Connected
};

class Session {
public:
    static Session& get();

    bool startHost();
    bool connectToPeer(const std::string& ip);
    void disconnect();

    void sendPacket(const std::vector<uint8_t>& packet);
    
    SessionState getState() const;
    std::string getConnectedIP() const;

    using PacketCallback = std::function<void(const PacketHeader&, const std::vector<uint8_t>&)>;
    using DisconnectCallback = std::function<void()>;
    
    void setPacketCallback(PacketCallback callback);
    void setDisconnectCallback(DisconnectCallback callback);

private:
    Session() = default;
    ~Session();

    void hostAcceptLoop();
    void receiveLoop();
    void flushSendQueue();

    std::atomic<SessionState> m_state{SessionState::Disconnected};
    int m_listenSock{-1};
    int m_peerSock{-1};
    std::string m_peerIp;

    std::thread m_acceptThread;
    std::thread m_receiveThread;
    std::thread m_sendThread;

    std::mutex m_sendMutex;
    std::vector<std::vector<uint8_t>> m_sendQueue;
    std::atomic<bool> m_hasDataToSend{false};

    PacketCallback m_packetCallback;
    DisconnectCallback m_disconnectCallback;
};

} // namespace network
