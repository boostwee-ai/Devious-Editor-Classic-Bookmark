#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <Geode/Geode.hpp>

namespace network {

struct DiscoveredPeer {
    std::string ip;
    std::string username;
    bool inEditor;
    bool collabEnabled;
    long long lastSeen; // in standard timer (e.g. tick/ms)
};

class Discovery {
public:
    static Discovery& get();

    bool start();
    void stop();

    void setLocalState(const std::string& username, bool inEditor, bool collabEnabled);
    std::vector<DiscoveredPeer> getActivePeers();

private:
    Discovery() = default;
    ~Discovery();

    void broadcastLoop();
    void listenLoop();

    std::atomic<bool> m_running{false};
    std::thread m_broadcastThread;
    std::thread m_listenThread;

    std::mutex m_stateMutex;
    std::string m_username;
    bool m_inEditor{false};
    bool m_collabEnabled{true};

    std::mutex m_peersMutex;
    std::map<std::string, DiscoveredPeer> m_peers; // Key is IP
    
    int m_listenSock{-1};
};

} // namespace network
