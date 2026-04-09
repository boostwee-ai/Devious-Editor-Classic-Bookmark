#include "Protocol.hpp"
#include <cstring>
#include <stdexcept>

namespace network {

std::vector<uint8_t> Protocol::createPacket(PacketType type, const std::vector<uint8_t>& payload) {
    if (payload.size() > 0xFFFF) {
        throw std::runtime_error("Payload too large");
    }

    std::vector<uint8_t> packet;
    packet.resize(sizeof(PacketHeader) + payload.size());

    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.data());
    header->version = CURRENT_VERSION;
    header->type = type;
    // Store in little-endian. x86/x64 are little-endian natively, so direct assignment works.
    // For full portability:
    // header->payloadLength = (payload.size() & 0xFF) | ((payload.size() >> 8) << 8);
    header->payloadLength = static_cast<uint16_t>(payload.size());

    if (!payload.empty()) {
        std::memcpy(packet.data() + sizeof(PacketHeader), payload.data(), payload.size());
    }

    return packet;
}

std::vector<uint8_t> Protocol::createPacket(PacketType type, const std::string& payload) {
    std::vector<uint8_t> payloadData(payload.begin(), payload.end());
    return createPacket(type, payloadData);
}

bool Protocol::parsePacket(std::vector<uint8_t>& buffer, PacketHeader& outHeader, std::vector<uint8_t>& outPayload) {
    if (buffer.size() < sizeof(PacketHeader)) {
        return false;
    }

    PacketHeader header;
    std::memcpy(&header, buffer.data(), sizeof(PacketHeader));

    if (buffer.size() < sizeof(PacketHeader) + header.payloadLength) {
        return false; // Not enough data yet
    }

    outHeader = header;
    outPayload.resize(header.payloadLength);
    if (header.payloadLength > 0) {
        std::memcpy(outPayload.data(), buffer.data() + sizeof(PacketHeader), header.payloadLength);
    }

    // Remove the processed packet from the buffer
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(PacketHeader) + header.payloadLength);

    return true;
}

} // namespace network
