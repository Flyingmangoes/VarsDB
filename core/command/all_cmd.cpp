#include <iostream>
#include "utilities.hpp"
#include "command.hpp"

void CommandManager::ECHO(int fd_dest, const char* message) {
    char echo_response[MAX_MESSAGE_BUFFER];
    snprintf(echo_response, sizeof(echo_response),"+%s\r\n", message);
    send(fd_dest, echo_response, strlen(echo_response), 0);
};

void CommandManager::PING(int fd_dest) {
    const char* response = "+PONG\r\n";
    send(fd_dest, response, strlen(response), 0);
};

void CommandManager::SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage) {
    if(opt != NULL && n > 0) {
        if(strncmp(opt, "EX", 2) == 0) {
            storage->Temporary[key] = make_expiry_value(value, std::chrono::seconds(n));
        }
        else if(strncmp(opt, "PX", 2) == 0) {
            storage->Temporary[key] = make_expiry_value(value, std::chrono::milliseconds(n));
        }
        else {
            send(fd_dest, "-ERR unknown option\r\n", 21, 0);
            return;
        }
    }
    else {
        struct StoreValue sv;
        sv.value      = std::string(value);
        sv.has_expiry = false;
        storage->Standard[key]     = sv;
    }

    send(fd_dest, "+OK\r\n", 5, 0);
};

void CommandManager::GET(int fd_dest, const char* key, struct storage_holder* storage) {
    char buf[MAX_MESSAGE_BUFFER];

    auto it = storage->Temporary.find(key);
    if(it != storage->Temporary.end()) {
        if(is_expired(it->second)) {
            storage->Temporary.erase(it);
            send(fd_dest, "$-1\r\n", 5, 0);
            return;
        }

        snprintf(buf, sizeof(buf), "+%s\r\n", it->second.value.c_str());
        send(fd_dest, buf, strlen(buf), 0);
        return;
    }

    auto it2 = storage->Standard.find(key);
    if (it2 == storage->Standard.end()) {
        send(fd_dest, "$-1\r\n", 5, 0);
        return;
    }

    snprintf(buf, sizeof(buf), "+%s\r\n", it2->second.value.c_str());
    send(fd_dest, buf, strlen(buf), 0);
};

void CommandManager::DEL(int fd_dest, char* argv, struct storage_holder* storage) {
    
};