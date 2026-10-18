#include <iostream>
#include "RESP.hpp"

#define CLI_PROMPT "REDIS >"
#define CLI_EXIT "EXIT"
#define RESP_MESSAGE_BUFFER 101

RESP::RESP(SOCKET socket_fd) {
    this->socket_fd = socket_fd;
};

int RESP::send_command(const char* cmd) {
    char buf[RESP_MESSAGE_BUFFER];
    snprintf(buf, sizeof(buf), "%s\n", cmd);

    if (send(this->socket_fd, buf, strlen(buf), 0) < 0) {
        vars::MessageFactory::error("Failed to send command", errno);
        return -1;
    }

    return 0;
};

int RESP::read_response() {
    char buf[RESP_MESSAGE_BUFFER] = {0};
    ssize_t count = recv(this->socket_fd, buf, sizeof(buf) - 1, 0);

    if (count < 0) {
        vars::MessageFactory::error("Failed to read response", errno);
        return -1;
    }

    if (count == 0) {
        vars::MessageFactory::warn("Disconnected from server");
        return -1;
    }

    buf[count] = '\0';
    std::cout<<buf<<std::endl;
    return 0;
};

void RESP::loop() {
    assert(this->socket_fd != -1);

    char buf[RESP_MESSAGE_BUFFER];
    while(1) {
        std::cout<<CLI_PROMPT<<" ";
        std::cin.getline(buf, sizeof(buf));

        if (std::cin.eof()) {
            std::cout << "Exiting environment" << std::endl;
            break;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore();
            continue;
        }

        if (strlen(buf) == 0) continue;

        if (strncmp(buf, CLI_EXIT, strlen(CLI_EXIT)) == 0) {
            std::cout<<"Exiting environment"<<std::endl;
            break;
        }

        if (send_command(buf)  != 0) break;
        if (read_response()    != 0) break;
    }   
};