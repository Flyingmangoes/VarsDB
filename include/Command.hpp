#ifndef COMMAND_H
#define COMMAND_H

#include <sys/socket.h>
#include <chrono>
#include <unordered_map>
#include "MemoryStore.hpp"
#include "TCP.hpp"

typedef enum {
    CMD_SERVER  = 1,
    CMD_CLIENT  = 2,
    CMD_UNKNOWN = 0
} cmd_type;

struct cmd_list {
    const char* name;
    cmd_type  target;    
};

extern struct cmd_list cmd_lists[];

namespace GeneralCommand {
    void ECHO(int fd_dest, const char* message);
    void PING(int fd_dest);
    void SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage);
    void GET(int fd_dest, const char* key, struct storage_holder* storage);
    void DEL(int fd_dest, int count, const char** keys, struct storage_holder* storage);
    void MOVE(int fd_dest, const char* key, const char* _dest, const char* opt, const int duration, struct storage_holder* storage);
    void EXISTS(int fd_dest, int count,  const char** keys, struct storage_holder* storage);
};

namespace StreamCommand {

};

namespace ListCommand {
    void RPUSH(int fd_dest, const char* key, const char* value, storage_holder* storage);
    void LPUSH(int fd_dest, const char* key, const char* value, storage_holder* storage);
    void LPOP(int fd_dest, const char* key, storage_holder* storage);
    void RPOP(int fd_dest, const char* key, storage_holder* storage);
    void LRANGE(int fd_dest, const char* key,int start, int stop, storage_holder* storage);
};

#endif