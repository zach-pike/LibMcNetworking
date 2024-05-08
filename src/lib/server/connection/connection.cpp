#include "connection.hpp"

#include <unistd.h>
#include "definitions.hpp"


NetworkConnection::NetworkConnection(int socketFileDescriptor, sockaddr sockAdd):
    socketDescriptor(socketFileDescriptor),
    socketAddress(sockAdd) {}
NetworkConnection::NetworkConnection(NetworkConnection&& old) {
    socketDescriptor = old.socketDescriptor;
    socketAddress = old.socketAddress;
    workerRunning = old.workerRunning;
    inQueue = old.inQueue;
    outQueue = old.outQueue;

    networkWorker = std::move(old.networkWorker);
    old.socketDescriptor = 0;
    old.workerRunning = false;
}
NetworkConnection::~NetworkConnection() {
    if (!workerRunning) return;

    workerRunning = false;
    networkWorker.join();
}

void NetworkConnection::_networkWorker() {
    std::uint8_t* inBuffer = new std::uint8_t[RECIEVE_BUFF_SIZE];
    std::size_t bufferByteCount = 0;

    CommunicationPacketHeader messageHeader;
    std::vector<std::uint8_t> messageContent;
    bool messageHeaderParsed = false;
    std::size_t messageContentStartOffset;

    while(workerRunning) {
        // Try to read packet in
        // Packets start with packet type and packet size
        // once we have read enough bytes for that we switch
        // to trying to recieve the message content
        ssize_t bytesJustRead = read(socketDescriptor, inBuffer + bufferByteCount, RECIEVE_BUFF_SIZE - bufferByteCount);

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

                    printf("packet recvd\n");
                    inQueue.push_back(CommunicationPacket(messageHeader.packetType, messageContent));
                    inQueueMutex.unlock();

                    messageHeaderParsed = false;

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
        if (hasNewPackets()) {
            outQueueMutex.lock();
            CommunicationPacket packetToSend = outQueue.front();
            outQueue.pop_front();
            outQueueMutex.unlock();

            std::uint16_t packetType = (std::uint16_t)packetToSend.getPacketType();
            std::vector<std::uint8_t> packetData = packetToSend.getPacketData();
            std::uint32_t packetDataLength = packetData.size();

            // Send messageType
            write(socketDescriptor, &packetType, sizeof(packetType));

            // Send message size
            write(socketDescriptor, &packetDataLength, sizeof(packetDataLength));

            // Write data
            write(socketDescriptor, packetData.data(), packetData.size());
        }
    }

    // Deallocate the inBuffer
    delete[] inBuffer;

    close(socketDescriptor);
}

bool NetworkConnection::hasNewPackets() const {
    return inQueue.size() > 0;
}

CommunicationPacket NetworkConnection::consumePacket() {
    inQueueMutex.lock();

    CommunicationPacket p = inQueue.front();
    inQueue.pop_front();

    inQueueMutex.unlock();

    return p;

}

void NetworkConnection::startWorker() {
    // Start thread
    networkWorker = std::thread(std::bind(&NetworkConnection::_networkWorker, this));
}