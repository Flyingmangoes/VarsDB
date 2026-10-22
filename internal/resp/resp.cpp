#include <iostream>
#include "RESP.hpp"

#define CLI_PROMPT "VARS >"
#define CLI_EXIT "EXIT"
#define RESP_MESSAGE_BUFFER 256

RESP::RESP(SOCKET client_fd) {
    this->client_fd = client_fd;
};

int RESP::add_storage(::Storage* storage) {
    this->storage = storage;
};

int RESP::send_request(const char* raw) {
    const char* encoded = RESP::encode_resp(raw);
    if (send(this->client_fd, encoded, strlen(encoded), 0) < 0) {
        vars::MessageFactory::error("Failed to send command", errno);
        return -1;
    }

    return 0;
};

int RESP::read_response() {
    char raw_buf[RESP_MESSAGE_BUFFER] = {0};
    ssize_t count = recv(this->client_fd, raw_buf, sizeof(raw_buf) - 1, 0);

    if (count < 0) {
        vars::MessageFactory::error("Failed to read response", errno);
        return -1;
    }

    if (count == 0) {
        vars::MessageFactory::warn("Disconnected from server");
        return -1;
    }

    raw_buf[count] = '\0';
    std::cout<<raw_buf<<std::endl;
    return 0;
};

void RESP::loop() {
    assert(this->client_fd != -1);

    char raw_buf[RESP_MESSAGE_BUFFER];
    while(1) {
        std::cout<<CLI_PROMPT<<" ";
        std::cin.getline(raw_buf, sizeof(raw_buf));

        if (std::cin.eof()) {
            std::cout << "Exiting environment" << std::endl;
            break;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore();
            continue;
        }

        if (strlen(raw_buf) == 0) continue;

        if (strncmp(raw_buf, CLI_EXIT, strlen(CLI_EXIT)) == 0) {
            std::cout<<"Exiting environment"<<std::endl;
            break;
        }

        if (send_request(raw_buf)  != 0) break;
        if (read_response()    != 0) break;
    }   
};

const char* RESP::encode_resp(const char* raw_buf){
    
};