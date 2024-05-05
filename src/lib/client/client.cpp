#include "client.hpp"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <functional>
#include <cstdint>
#include <cctype>

#ifndef RECIEVE_BUFF_SIZE
#define RECIEVE_BUFF_SIZE 2048
#endif

NetworkClient::NetworkClient() {}
NetworkClient::~NetworkClient() {}

// Runs in seperate thread
void NetworkClient::_threadedWorker() {
    std::uint8_t* inBuffer = new std::uint8_t[RECIEVE_BUFF_SIZE];
    std::size_t bufferIndex = 0;

    while(workerShouldRun) {
        // Try to read packet in
        // Packets start with packet type and packet size
        // once we have read enough bytes for that we switch
        // to trying to recieve the message content
        ssize_t bytesJustRead = read(socketFileDescriptor, inBuffer + bufferIndex, RECIEVE_BUFF_SIZE - bufferIndex);

        // If we have read some bytes do something with them
        if (bytesJustRead > 0) {
            bufferIndex += bytesJustRead;
        }

        // See if we have enough bytes to parse the header
        if (bufferIndex > (sizeof(std::uint16_t) + sizeof(std::uint32_t))) {
                
        }
    }

    delete[] inBuffer;
}

void NetworkClient::startNetworking(std::string ipAddress, std::uint16_t port) {
    // Create socket and connect
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    // Create socket address
    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &socketAddress.sin_addr);

    // Connect to address
    if (connect(socketFileDescriptor, (sockaddr*)&socketAddress, sizeof(sockaddr_in)) < 0) {
        // Connection Failed
    }

    // If sucessfully connected start worker thread
    worker = std::thread(std::bind(&NetworkClient::_threadedWorker, this));
}