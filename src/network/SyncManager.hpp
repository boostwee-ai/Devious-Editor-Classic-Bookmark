#pragma once

#include "Protocol.hpp"
#include "../editor/ActionSerializer.hpp"
#include <vector>
#include <string>

class LevelEditorLayer;

namespace network {

class SyncManager {
public:
    static SyncManager& get();

    void setupCallbacks();

    void sendAction(editor::ActionType type, const std::string& serializedData);
    void sendViewportPosition(float x, float y, float zoom);
    void sendLevelSync(); // Could pass string here
    void sendSessionLeave();

    void handleIncomingPacket(const PacketHeader& header, const std::vector<uint8_t>& payload);

    bool isSyncing() const { return m_internalSync; }
    void setInternalSync(bool active) { m_internalSync = active; }

    float m_remoteViewportX{0.f};
    float m_remoteViewportY{0.f};
    float m_remoteZoom{1.f};

private:
    SyncManager() = default;
    
    // To prevent infinite broadcast loops when receiving an edit and applying it
    bool m_internalSync{false};
};

} // namespace network
