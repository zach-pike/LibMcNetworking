#pragma once

#include <cstdint>
#include <vector>

class CommunicationPacket {
public:
    enum class PacketType : std::uint16_t {
        HANDSHAKE,          // Sent by client
        HANDSHAKE_RESP,     // Sent by server

        HEARTBEAT,          // Sent by server
        HEARTBEAT_RESP,     // Sent by client

        BASIC_MESSAGE,          // Sent by client
        BASIC_MESSAGE_RESPONSE  // Sent by server
    };
private:
    PacketType packetType;
    std::vector<std::uint8_t> messageData;
public:
    CommunicationPacket(PacketType type, std::uint8_t* data, std::uint32_t size);
    ~CommunicationPacket();

    std::vector<std::uint8_t> serialize() const;
    static CommunicationPacket parseFromVec(std::vector<std::uint8_t> data);
};