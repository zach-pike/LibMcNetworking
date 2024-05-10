#include "client/client.hpp"

int main() {
    NetworkClient client;

    client.startNetworking("0.0.0.0", 8000);

    const char* mystr = "hello world";
    std::uint8_t* d = (std::uint8_t*)mystr;
    size_t s = 11;

    client.sendMessage(CommunicationPacket(CommunicationPacket::PacketType::BASIC_MESSAGE, d, s)); 

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    client.stopNetworking();
}
