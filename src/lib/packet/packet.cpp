#include "packet.hpp"

CommunicationPacket::CommunicationPacket(PacketType type, std::uint8_t* data, std::uint32_t size) {
    packetType = type;
    messageData.assign(data, data + size);
}

CommunicationPacket::~CommunicationPacket() {}

std::vector<std::uint8_t> CommunicationPacket::serialize() const {
    // Packet structure is as follows
    /*  | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 13 | 14 | 15 |
        +-------------------------------------------------------------------------------+
        |                                   Packet Type                                 |
        +-------------------------------------------------------------------------------+
        |                                   Message Size                                |
        |                                                                               |
        +-------------------------------------------------------------------------------+
        /                                   Message content                             /
        /                                                                               /
        /                                                                               /
        +-------------------------------------------------------------------------------+
    */

    std::vector<std::uint8_t> packetData;

    // Insert packet type
    std::uint16_t packetTypeInt = (std::uint16_t)packetType;
    packetData.insert(packetData.end(), &packetTypeInt, &packetTypeInt + sizeof(packetTypeInt));

    // Insert message size
    std::uint32_t messageSize = messageData.size();
    packetData.insert(packetData.end(), &messageSize, &messageSize + sizeof(messageSize));

    // Insert message
    packetData.insert(packetData.end(), messageData.begin(), messageData.end());

    return packetData;
}

CommunicationPacket CommunicationPacket::parseFromVec(std::vector<std::uint8_t> data) {
    return CommunicationPacket(CommunicationPacket::PacketType::BASIC_MESSAGE, nullptr, 0);
}