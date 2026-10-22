#include "TCP.hpp"

int SetFlag::set_socket_non_block(SOCKET _fd) {
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::setAddress(int port) {
    if (port <= 0 || port > 65535 ) {
        vars::MessageFactory::warn("Not provided a port using default port");
        port = DEFAULT_PORT;
    }

    if (port < 1024) {
        vars::MessageFactory::warn("Port below 1024 required root privileges");
    }

    memset(&this->address, 0, sizeof(this->address));

    this->address.sin_family = this->domain;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(port);

    this->port = port;
    vars::MessageFactory::info("Address configured");
};

void Server::setClient(SOCKET client_fd, char* argv) {
    this->arguments = argv;
    this->client_fd = client_fd;
}

void Server::setStorage(Storage* storage) {
    this->storage = storage;
};

SOCKET Client::getSocket() {
    return this->socket_fd;
};

void Client::setAddress(int port) {
    if (port <= 0 || port > 65535 ) {
        vars::MessageFactory::warn("Not provided a port using default port");
        port = DEFAULT_PORT;
    }

    memset(&this->address, 0, sizeof(this->address));

    this->address.sin_family = this->domain;
    this->address.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &this->address.sin_addr) <= 0) {
        vars::MessageFactory::error("Invalid server address", errno);
        return;
    }

    this->port = port;
    vars::MessageFactory::info("Address configured");
};