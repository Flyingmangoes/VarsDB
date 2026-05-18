#ifndef RESP_H
#define RESP_H

#include "utilities.hpp"
#include "network.hpp"

#define CLI_PROMPT "REDIS >"
#define CLI_EXIT "EXIT"

class CLI_Manager {
private:
    SOCKET socket_fd;

    int send_command(const char* cmd);
    int read_response();
public:
    CLI_Manager(SOCKET socket_fd);
    void loop();
};

#endif