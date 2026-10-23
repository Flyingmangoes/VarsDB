#ifndef COMMAND_H
#define COMMAND_H

#include <sys/socket.h>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include "MemoryStore.hpp"

enum StartupType {
    CMD_SERVER  = 1,
    CMD_CLIENT  = 2,
    CMD_UNKNOWN = 0
};

struct StartupList {
    const char* name;        
    StartupType target;
};

extern StartupList startup_list[];

namespace TYPECommand {
    void INT(int fd_dest, const std::string& value);
    void STR(int fd_dest, const std::string& value);
    void DOUBLE(int fd_dest, const std::string& value);
};

class GeneralCommand {
private:
    Storage* storage;
public:
    GeneralCommand(Storage* storage);

    void ECHO(int fd_dest, const std::string& value);
    void PING(int fd_dest);
    void SET(int fd_dest, const std::string& key, const std::string& value, const std::string& opt, int n);
    void GET(int fd_dest, const std::string& key);
    void DEL(int fd_dest, const std::vector<std::string>& keys);     
    void MOVE(int fd_dest, const std::string& key, const std::string& storage_dest, const std::string& opt, int duration);
    void EXISTS(int fd_dest, const std::vector<std::string>& keys);
};

class StreamCommand {
private:
    Storage* storage;
public:
    StreamCommand(Storage* storage) : storage(storage) {} 
};

class ListCommand {
private:
    Storage* storage;
public:
    ListCommand(Storage* storage) : storage(storage) {}
    void RPUSH(int fd_dest, const std::string& key, const std::string& value);
    void LPUSH(int fd_dest, const std::string& key, const std::string& value);
    void LPOP(int fd_dest, const std::string& key);
    void RPOP(int fd_dest, const std::string& key);
    void LRANGE(int fd_dest, const std::string& key, int start, int stop);
};

#endif