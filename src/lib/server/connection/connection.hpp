#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <deque>

#include "packet/packet.hpp"

class NetworkConnection {
private:
    std::thread networkWorker;
    int socketDescriptor;
    bool workerRunning = true;

    std::deque<CommunicationPacket> inQueue;
    std::mutex inQueueMutex;

    std::deque<CommunicationPacket> outQueue;
    std::mutex outQueueMutex;

    void _networkWorker();
public:
    NetworkConnection();
    ~NetworkConnection();
};