#ifndef COMMAND_H
#define COMMAND_H

#include <sys/socket.h>
#include <chrono>
#include <unordered_map>

#include "store.hpp"

class GeneralCommand {
public:
    static void ECHO(int fd_dest, const char* message);
    static void PING(int fd_dest);
    static void SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage);
    static void GET(int fd_dest, const char* key, struct storage_holder* storage);
    static void DEL(int fd_dest, int count, const char** keys, struct storage_holder* storage);
    static void MOVE(int fd_dest, const char* key, const char* _dest, struct storage_holder* storage);
    static void EXISTS(int fd_dest, int count,  const char** keys, struct storage_holder* storage);
};

class StreamCommand {
public:

};

class ListCommand {
public:
    static void RPUSH (int fd_dest, const char* key, const char** value, int count, storage_holder* storage);
    static void LPUSH (int fd_dest, const char* key, const char** value, int count, storage_holder* storage);
    static void LPOP  (int fd_dest, const char* key, storage_holder* storage);
    static void RPOP  (int fd_dest, const char* key, storage_holder* storage);
    static void LRANGE (int fd_dest, const char* key,int start, int stop, storage_holder* storage);
};

#endif