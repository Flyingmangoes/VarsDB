#include <iostream>
#include "server.hpp"

Client::Client(int domain, int type, int protocol) {
        this->domain   = domain;
        this->type     = type;
        this->protocol = protocol;
        this->port     = -1;
        this->socket_fd = -1;
};

void Client::setAddress(int port) {
    if (port <= 0 || port > 65535 ) {
        MessageManager::warn("Not provided a port using default port");
        port = DEFAULT_PORT;
    }

    memset(&this->address, 0, sizeof(this->address));

    this->address.sin_family = this->domain;
    this->address.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &this->address.sin_addr) <= 0) {
        MessageManager::error("Invalid server address", errno);
        return;
    }

    this->port = port;
    MessageManager::info("Address configured");
};

SOCKET Client::getSocket() {
    return this->socket_fd;
};

int Client::setup_client() {
    this->socket_fd = socket(this->domain, this->type, this->protocol);
    if (this->socket_fd < 0) {
        MessageManager::error("Failed to set socket", errno);
        return -1;
    }

    MessageManager::info("Client Setup Complete");
    return 0;
};

int Client::connect_to_server() {
    if (connect(this->socket_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        MessageManager::error("Socket failed to connect", errno);
        return -1;
    }

    char msg[MAX_MESSAGE_BUFFER];
    snprintf(msg, sizeof(msg), "Connected to server on port %d", this->port);
    MessageManager::info(msg);
    
    return 0;
}

