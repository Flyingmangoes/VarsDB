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
    CMD_HELP    = 3,
    CMD_UNKNOWN = 0
} CmdType;

typedef struct CmdLists {
    const char* name;
    CmdType  target;    
};

extern CmdLists cmd_lists[];

namespace TYPECommand {
    void INT(int fd_dest, const char* value);
    void STR(int fd_dest, const char* value);
    void DOUBLE(int fd_dest, const char* value);
};

class GeneralCommand {
private:
    Storage* storage;
public:
    GeneralCommand(Storage* storage);

    void ECHO(int fd_dest, const char* message);
    void PING(int fd_dest);
    void SET(int fd_dest, const char* key, const char* value, const char* opt, int n);
    void GET(int fd_dest, const char* key);
    void DEL(int fd_dest, int count, const char** keys);
    void MOVE(int fd_dest, const char* key, const char* _dest, const char* opt, const int duration);
    void EXISTS(int fd_dest, int count,  const char** keys);
};

class StreamCommand {
private:
    Storage* storage;
public:
    StreamCommand(Storage* storage);
};

class ListCommand {
private:
    Storage* storage;
public:
    ListCommand(Storage* storage);
    void RPUSH(int fd_dest, const char* key, const char* value);
    void LPUSH(int fd_dest, const char* key, const char* value);
    void LPOP(int fd_dest, const char* key);
    void RPOP(int fd_dest, const char* key);
    void LRANGE(int fd_dest, const char* key,int start, int stop);
};

#endif