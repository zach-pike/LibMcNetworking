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
NetworkServer::~NetworkServer() {}

void NetworkServer::_listenerThread() {
    while (workerRunning) {
        
    }
}

void NetworkServer::startNetworking(std::string bindIP, std::uint16_t bindPort) {
    // Pretty much same as client
    serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    // Reuse socket port
    if (setsockopt(serverFileDescriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Create socket address
    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &socketAddress.sin_addr);

    // Bind socket
    if (bind(serverFileDescriptor, (sockaddr*)&socketAddress, sizeof(sockaddr_in)) < 0) {
        // Error
    }

    // Listen on socket
    if (listen(serverFileDescriptor, SERVER_MAX_LISTEN_BAGLOG) < 0) {
        // Error
    }

    // Make socket nonblocking
    setSocketBlockingEnabled(serverFileDescriptor, false);

    // If sucessfully connected start listener thread
    listenerThread = std::thread(std::bind(&NetworkServer::_listenerThread, this));

}