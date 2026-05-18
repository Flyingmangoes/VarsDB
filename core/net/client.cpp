#include <iostream>
#include "network.hpp"

Client::Client(int domain, int type, int protocol) {
        this->domain   = domain;
        this->type     = type;
        this->protocol = protocol;
        this->port     = -1;
        this->socket_fd = -1;
};

int Client::setup_client() {
    this->socket_fd = socket(this->domain, this->type, this->protocol);
    if (this->socket_fd < 0) {
        utils::MessageManager::error("Failed to set socket", errno);
        return -1;
    }

    utils::MessageManager::info("Client Setup Complete");
    return 0;
};

int Client::connect_to_server() {
    if (connect(this->socket_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        utils::MessageManager::error("Socket failed to connect", errno);
        return -1;
    }

    char msg[MAX_MESSAGE_BUFFER];
    snprintf(msg, sizeof(msg), "Connected to server on port %d", this->port);
    utils::MessageManager::info(msg);
    
    return 0;
}

