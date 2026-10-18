#ifndef RESP_H
#define RESP_H

#include "TCP.hpp"
#include "Command.hpp"
#include "Message.hpp"

class RESP {
private:
    SOCKET socket_fd;

    int send_command(const char* cmd);
    int read_response();
public:
    RESP(SOCKET socket_fd);
    void loop();
};

class RESP_Parser {
public:
    static cmd_type parse_cmd(const char* input);
    static storage_type parse_storage(const char* input);
    template <typename TList, typename TResult> static TResult parse_table(const char* input, TList* table, TResult unknown){
        if(input == NULL) return unknown;

        char lower[32] = {0};
        size_t len = strlen(input);
        if(len < sizeof(lower)) len = sizeof(lower) - 1;
    
        for (size_t i=0; i < len;i++) {
            lower[i] = tolower(input[i]);
        }
        lower[len] = '\0';

        for(int i=0;table[i].name != NULL; i++) {
            if(strncmp(lower, table[i].name, strlen(table[i].name)) == 0) {
                return table[i].target;
            }
        }

        return unknown;
    };
};

#endif