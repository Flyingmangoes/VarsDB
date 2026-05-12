#ifndef COMMAND_H
#define COMMAND_H

#include <sys/socket.h>
#include <chrono>
#include <unordered_map>

#include "store.hpp"

class CommandManager {
public:
    static void ECHO(int fd_dest, const char* message);
    static void PING(int fd_dest);
    static void SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage);
    static void GET(int fd_dest, const char* key, struct storage_holder* storage);
    static void DEL(int fd_dest, char* argv, struct storage_holder* storage);
    static void MOVE(int fd_dest, const char* key, struct storage_holder* storage);
    static void EXISTS(int fd_dest, char *keys, struct storage_holder* storage);
};

class Stream : public CommandManager {
public:

};

class List : public CommandManager {
public:

};

#endif