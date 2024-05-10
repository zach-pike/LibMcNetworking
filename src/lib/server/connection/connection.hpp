#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <deque>

#include "packet/packet.hpp"

#include <netinet/in.h>

class NetworkConnection {
private:
    std::thread networkWorker;
    int socketDescriptor;
    sockaddr socketAddress;
    bool workerRunning = true;

    std::deque<CommunicationPacket> inQueue;
    std::mutex inQueueMutex;

    std::deque<CommunicationPacket> outQueue;
    std::mutex outQueueMutex;

    void _networkWorker();
public:
    NetworkConnection(int socketFileDescriptor, sockaddr socketAddress);
    NetworkConnection(NetworkConnection&& old);
    ~NetworkConnection();

    bool hasNewPackets() const;
    CommunicationPacket consumePacket();

    int getSocketDescriptor() const;

    void startWorker();
};