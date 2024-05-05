#pragma once

#include <cstdint>
#include <vector>

class CommunicationPacket {
public:
    enum class PacketType : std::uint16_t {
        HANDSHAKE = 0,                // Sent by client
        HANDSHAKE_RESP = 1,           // Sent by server

        HEARTBEAT = 2,                // Sent by server
        HEARTBEAT_RESP = 3,           // Sent by client

        BASIC_MESSAGE = 4,            // Sent by client
        BASIC_MESSAGE_RESPONSE = 5    // Sent by server
    };
private:
    PacketType packetType;
    std::vector<std::uint8_t> messageData;
public:
    CommunicationPacket(PacketType type, std::uint8_t* data, std::uint32_t size);
    CommunicationPacket(PacketType type, std::vector<std::uint8_t> data);
    ~CommunicationPacket();

    PacketType getPacketType() const;
    std::vector<std::uint8_t> getPacketData() const;

    std::vector<std::uint8_t> serialize() const;
    static CommunicationPacket parseFromVec(std::vector<std::uint8_t> data);
};

struct CommunicationPacketHeader {
    CommunicationPacket::PacketType packetType;
    std::uint32_t packetLength;
} __attribute__((packed));

// Packet types