#ifndef CLI_H
#define CLI_H

#include "utilities.hpp"
#include "server.hpp"

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