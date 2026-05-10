#include <iostream>
#include "message.hpp"
#include "parser.hpp"

struct target_list Parser::targets[] = {
    {"server", CMD_SERVER},
    {"client", CMD_CLIENT},
    {NULL, CMD_UNKNOWN}
};

Parser::Parser(int argc, char** argv) {
    this->argc = argc;
    this->argv = argv;
};

cmd_target Parser::parse() {
    if(this->argc < 2) {
        MessageManager::error("No command provided");
        return CMD_UNKNOWN;
    }

    char lower[32] = {0};
    size_t len = strlen(this->argv[1]);

    if (len >= sizeof(lower)) len = sizeof(lower) - 1;

    for (size_t i = 0;i < len; i++) {
        lower[i] = tolower(this->argv[1][i]);
    }

    lower[len] = '\0';

    for (int i = 0; targets[i].name != NULL; i++) {
        if(strncmp(lower, targets[i].name, strlen(targets[i].name)) == 0) {
            return targets[i].target;
        }
    }

    MessageManager::error("Unknown command");
    return CMD_UNKNOWN;
}; 