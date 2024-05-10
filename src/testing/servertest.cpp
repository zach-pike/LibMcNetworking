#include "server/server.hpp"

#include <iostream>

int main() {
    NetworkServer ns;

    ns.startNetworking("0.0.0.0", 8000);

    printf("Waiting for connection\n");
    while (ns.connectedCount() < 1);

    printf("Waiting for packet\n");
    while (ns.clients()[0]->hasNewPackets() != true);

    auto newPacket = ns.clients()[0]->consumePacket();
    auto data = newPacket.getPacketData();

    std::string dataStr(data.begin(), data.end());

    std::cout << dataStr << std::endl;

    // TODO / wait for message packet and print it

    ns.stopNetworking();

}