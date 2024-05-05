#pragma once

#include "packet/packet.hpp"

#include <thread>
#include <string>
#include <queue>
#include <mutex>

class NetworkClient {
private:
    bool workerShouldRun = true;
    std::thread worker;

    // Socket
    int socketFileDescriptor;

    std::queue<CommunicationPacket> inQueue;
    std::mutex inQueueMutex;

    std::queue<CommunicationPacket> outQueue;
    std::mutex outQueueMutex;

    void _threadedWorker();
public:
    NetworkClient();
    ~NetworkClient();

    void startNetworking(std::string ipAddress, std::uint16_t port);
};