#include "MemoryStore.hpp"
#include "Command.hpp"

ListCommand::ListCommand(Storage* storage){
    this->storage = storage;
};

void ListCommand::RPUSH(int fd_dest, const char* key, const char* value) {
    auto it = this->storage->db.find(key);
    if(it != this->storage->db.end()) {   
        it->second.items.push_front(std::string(value));

        send(fd_dest, "+OK\r\n", 5, 0);
        vars::MessageFactory::debug("RPUSH update list");
        return;
    }
    
    struct StoreValue sv;
    sv.items.push_back(std::string(value));
    sv.has_expiry = false;
    this->storage->db[key] = sv;

    send(fd_dest, "+OK\r\n", 5, 0);
    vars::MessageFactory::debug("RPUSH list created");
    return;
};

void ListCommand::LPUSH(int fd_dest, const char* key, const char* value) {
auto it = this->storage->db.find(key);
    if(it != this->storage->db.end()) {   
        it->second.items.push_back(std::string(value));

        send(fd_dest, "+OK\r\n", 5, 0);
        vars::MessageFactory::debug("LPUSH update list");
        return;
    }

    struct StoreValue sv;
    sv.items.push_front(std::string(value));
    sv.has_expiry = false;
    this->storage->db[key] = sv;

    send(fd_dest, "+OK\r\n", 5, 0);
    vars::MessageFactory::debug("LPUSH list created");
    return;
};

void ListCommand::LPOP(int fd_dest, const char* key){
    auto it = this->storage->db.find(key);
    if(it == this->storage->db.end()){
        send(fd_dest, "$-1\r\n", 5, 0);
        return;
    };

    vars::MessageFactory::debug("LPOP Success");
    return;
};

void ListCommand::RPOP(int fd_dest, const char* key) {
    vars::MessageFactory::debug("RPOP Success");
    return;
};

void ListCommand::LRANGE (int fd_dest, const char* key,int start, int stop) {  
    vars::MessageFactory::debug("LRANGE Success");
    return;
};