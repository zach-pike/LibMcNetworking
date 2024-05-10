#pragma once

#include "packet/packet.hpp"
#include "connection/connection.hpp"

#include <thread>
#include <mutex>
#include <string>
#include <cstdint>
#include <vector>

class NetworkServer {
private:
    std::thread listenerThread;
    bool workerRunning = true;
    int serverFileDescriptor;

    std::vector<std::unique_ptr<NetworkConnection>> connections;
    std::mutex connectionsMutex;

    void _listenerThread();
public:
    NetworkServer();
    ~NetworkServer();

    std::vector<std::unique_ptr<NetworkConnection>>& clients();

    std::size_t connectedCount() const;

    void startNetworking(std::string bindIP, std::uint16_t bindPort);

    void stopNetworking();
};