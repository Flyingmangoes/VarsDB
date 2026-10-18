#include "MemoryStore.hpp"
#include "Command.hpp"

void ListCommand::RPUSH(int fd_dest, const char* key, const char* value, storage_holder* storage) {
    auto it = storage->db.find(key);
    if(it != storage->db.end()) {   
        it->second.items.push_front(std::string(value));

        send(fd_dest, "+OK\r\n", 5, 0);
        vars::MessageFactory::debug("RPUSH update list");
        return;
    }
    
    struct StoreValue sv;
    sv.items.push_back(std::string(value));
    sv.has_expiry = false;
    storage->db[key] = sv;

    send(fd_dest, "+OK\r\n", 5, 0);
    vars::MessageFactory::debug("RPUSH list created");
    return;
};

void ListCommand::LPUSH(int fd_dest, const char* key, const char* value, storage_holder* storage) {
auto it = storage->db.find(key);
    if(it != storage->db.end()) {   
        it->second.items.push_back(std::string(value));

        send(fd_dest, "+OK\r\n", 5, 0);
        vars::MessageFactory::debug("LPUSH update list");
        return;
    }

    struct StoreValue sv;
    sv.items.push_front(std::string(value));
    sv.has_expiry = false;
    storage->db[key] = sv;

    send(fd_dest, "+OK\r\n", 5, 0);
    vars::MessageFactory::debug("LPUSH list created");
    return;
};

void ListCommand::LPOP(int fd_dest, const char* key, storage_holder* storage){
    auto it = storage->db.find(key);
    if(it == storage->db.end()){
        send(fd_dest, "$-1\r\n", 5, 0);
        return;
    };

    vars::MessageFactory::debug("LPOP Success");
    return;
};

void ListCommand::RPOP(int fd_dest, const char* key, storage_holder* storage) {
    vars::MessageFactory::debug("RPOP Success");
    return;
};

void ListCommand::LRANGE (int fd_dest, const char* key,int start, int stop, storage_holder* storage) {  
    vars::MessageFactory::debug("LRANGE Success");
    return;
};