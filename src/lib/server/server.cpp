#include "server.hpp"

#include <functional>

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#ifndef SERVER_MAX_LISTEN_BAGLOG
#define SERVER_MAX_LISTEN_BAGLOG 10
#endif

static bool setSocketBlockingEnabled(int fd, bool blocking) {
   if (fd < 0) return false;

   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0);
}

NetworkServer::NetworkServer() {}
NetworkServer::~NetworkServer() {
    if (!workerRunning) return;

    workerRunning = false;
    listenerThread.join();
}

void NetworkServer::_listenerThread() {
    while (workerRunning) {
        // Try to accept a connection
        sockaddr newConnection;
        socklen_t sockLen = sizeof(newConnection);

        int clientSocket;
        if ((clientSocket = accept(serverFileDescriptor, &newConnection, &sockLen)) > 0) {
            // Create new connection
            NetworkConnection connection(clientSocket, newConnection);
            connections.push_back(std::move(connection));
            connections.back().startWorker();

            // connections.emplace(connections.end(), clientSocket, newConnection);
        }
    }

    close(serverFileDescriptor);
}

std::vector<NetworkConnection>& NetworkServer::clients() {
    return connections;
}

std::size_t NetworkServer::connectedCount() const {
    return connections.size();
}

void NetworkServer::startNetworking(std::string bindIP, std::uint16_t bindPort) {
    // Pretty much same as client
    serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    // Reuse socket port
    if (setsockopt(serverFileDescriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        printf("setsockopt\n");
        exit(1);
    }

    // Create socket address
    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(bindPort);
    inet_pton(AF_INET, bindIP.c_str(), &socketAddress.sin_addr);

    // Bind socket
    if (bind(serverFileDescriptor, (sockaddr*)&socketAddress, sizeof(sockaddr_in)) < 0) {
        // Error
        printf("Bind Error\n");
        exit(1);
    }

    // Listen on socket
    if (listen(serverFileDescriptor, SERVER_MAX_LISTEN_BAGLOG) < 0) {
        // Error
        printf("Listen Error\n");
        exit(1);
    }

    // Make socket nonblocking
    setSocketBlockingEnabled(serverFileDescriptor, false);

    // If sucessfully connected start listener thread
    listenerThread = std::thread(std::bind(&NetworkServer::_listenerThread, this));
}

void NetworkServer::stopNetworking() {
    workerRunning = false;
    listenerThread.join();
}