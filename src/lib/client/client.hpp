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

    std::deque<CommunicationPacket> inQueue;
    std::mutex inQueueMutex;

    std::deque<CommunicationPacket> outQueue;
    std::mutex outQueueMutex;

    void _threadedWorker();
public:
    NetworkClient();
    ~NetworkClient();

    void startNetworking(std::string ipAddress, std::uint16_t port);
    void stopNetworking();

    void sendMessage(CommunicationPacket packet);
};