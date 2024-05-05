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

#include <fcntl.h>

static bool setSocketBlockingEnabled(int fd, bool blocking) {
   if (fd < 0) return false;

   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0);
}

NetworkClient::NetworkClient() {}
NetworkClient::~NetworkClient() {}

// Runs in seperate thread
void NetworkClient::_threadedWorker() {
    std::uint8_t* inBuffer = new std::uint8_t[RECIEVE_BUFF_SIZE];
    std::size_t bufferByteCount = 0;

    CommunicationPacketHeader messageHeader;
    std::vector<std::uint8_t> messageContent;
    bool messageHeaderParsed = false;
    std::size_t messageContentStartOffset;

    while(workerShouldRun) {
        // Try to read packet in
        // Packets start with packet type and packet size
        // once we have read enough bytes for that we switch
        // to trying to recieve the message content
        ssize_t bytesJustRead = read(socketFileDescriptor, inBuffer + bufferByteCount, RECIEVE_BUFF_SIZE - bufferByteCount);

        // If we have read some bytes do something with them
        if (bytesJustRead > 0) {
            bufferByteCount += bytesJustRead;

            // Start reading content in
            if (messageHeaderParsed) {
                std::size_t bytesToReadIn = bufferByteCount - messageContentStartOffset;

                // Copy the portion in to the vector
                messageContent.insert(messageContent.end(), 
                    inBuffer + messageContentStartOffset,
                    inBuffer + messageContentStartOffset + bytesToReadIn
                );

                // If we have read all data nessesary then we will stop
                if (messageContent.size() >= messageHeader.packetLength) {
                    inQueueMutex.lock();
                    inQueue.push_back(CommunicationPacket(messageHeader.packetType, messageContent));
                    inQueueMutex.unlock();

                    messageContent.clear();
                }

                // Set the buffer write pointer back to the beginning
                bufferByteCount = 0;
            
                // Start reading from beginning on next read pass
                messageContentStartOffset = 0;
            }
        }

        // See if we have enough bytes to parse the header
        if (bufferByteCount >= sizeof(CommunicationPacketHeader) && !messageHeaderParsed) {
            // Parse header
            messageHeader = *(CommunicationPacketHeader*)inBuffer;

            // Start reading message content
            messageHeaderParsed = true;

            // Set initial read position
            messageContentStartOffset = sizeof(CommunicationPacketHeader);
        }

        // Consume message from queue and send it
        outQueueMutex.lock();
        CommunicationPacket packetToSend = outQueue.front();
        outQueue.pop_front();
        outQueueMutex.unlock();

        std::uint16_t packetType = (std::uint16_t)packetToSend.getPacketType();
        std::vector<std::uint8_t> packetData = packetToSend.getPacketData();
        std::uint32_t packetDataLength = packetData.size();

        // Send messageType
        write(socketFileDescriptor, &packetType, sizeof(packetType));

        // Send message size
        write(socketFileDescriptor, &packetDataLength, sizeof(packetDataLength));

        // Write data
        write(socketFileDescriptor, packetData.data(), packetData.size());
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
        printf("Error connecting");
        exit(0);
    }

    setSocketBlockingEnabled(socketFileDescriptor, false);

    // If sucessfully connected start worker thread
    worker = std::thread(std::bind(&NetworkClient::_threadedWorker, this));
}

void NetworkClient::stopNetworking() {
    workerShouldRun = false;
    worker.join();
}

void NetworkClient::sendMessage(CommunicationPacket packet) {
    outQueueMutex.lock();
    outQueue.push_back(packet);
    outQueueMutex.unlock();
}

