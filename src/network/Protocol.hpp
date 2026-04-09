#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace network {

enum class PacketType : uint8_t {
    Heartbeat = 1,
    CollabRequest = 2,
    CollabResponse = 3,
    EditorAction = 4,
    PlayerPositionUpdate = 5,
    SessionLeave = 6,
    LevelSync = 7
};

#pragma pack(push, 1)
struct PacketHeader {
    uint8_t version;
    PacketType type;
    uint16_t payloadLength;
};
#pragma pack(pop)

class Protocol {
public:
    static constexpr uint8_t CURRENT_VERSION = 1;
    
    static std::vector<uint8_t> createPacket(PacketType type, const std::vector<uint8_t>& payload);
    static std::vector<uint8_t> createPacket(PacketType type, const std::string& payload);
    
    static bool parsePacket(std::vector<uint8_t>& buffer, PacketHeader& outHeader, std::vector<uint8_t>& outPayload);
};

} // namespace network
