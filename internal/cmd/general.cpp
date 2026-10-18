#include "Command.hpp"

#define RESPONSE_BUFFER 216

void GeneralCommand::ECHO(int fd_dest, const char* message) {
    char echo_response[RESPONSE_BUFFER];
    snprintf(echo_response, sizeof(echo_response),"+%s\r\n", message);
    send(fd_dest, echo_response, strlen(echo_response), 0);
    vars::MessageFactory::debug("ECHO Success");
    return;
};

void GeneralCommand::PING(int fd_dest) {
    const char response[RESPONSE_BUFFER] = "+PONG\r\n";
    send(fd_dest, response, strlen(response), 0);
    vars::MessageFactory::debug("PING Success");
    return;
};

void GeneralCommand::SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage) {
    if(opt != NULL && n > 0) {
        if(strncmp(opt, "EX", 2) == 0) {
            storage->db[key] = vars::MemoryStorage::make_expiry_value(value, std::chrono::seconds(n));
        }
        else if(strncmp(opt, "PX", 2) == 0) {
            storage->db[key] = vars::MemoryStorage::make_expiry_value(value, std::chrono::milliseconds(n));
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
        sv.type = STANDARD;
        storage->db[key] = sv;
    }

    send(fd_dest, "+OK\r\n", 5, 0);
    vars::MessageFactory::debug("SET Success");
    return;
};

void GeneralCommand::GET(int fd_dest, const char* key, struct storage_holder* storage) {
    char buf[RESPONSE_BUFFER];

    auto it = storage->db.find(key);
    if(it == storage->db.end()) {
        send(fd_dest, "$-1\r\n", 5, 0);
        return;
    }

    if(it->second.type == cmp_temp()){
        if(vars::MemoryStorage::is_expired(it->second)) {
            storage->db.erase(it);
            send(fd_dest, "$-1\r\n", 5, 0);
            return;
        }
    }

    snprintf(buf, sizeof(buf), "+%s\r\n", it->second.value.c_str());
    send(fd_dest, buf, strlen(buf), 0);
    vars::MessageFactory::debug("GET Success");
    return;
};

void GeneralCommand::DEL(int fd_dest, int count, const char** keys, struct storage_holder* storage) {
    char response[RESPONSE_BUFFER];
    int deleted = 0;

    for (int i = 0; i < count; i++) {
        deleted += storage->db.erase(keys[i]);
    }

    snprintf(response, sizeof(response), ":%d\r\n", deleted);
    send(fd_dest, response, strlen(response), 0);
};

void GeneralCommand::EXISTS(int fd_dest, int count, const char** keys, struct storage_holder* storage) {
    char buf[RESPONSE_BUFFER];
    int exists = 0;

    for (int i = 0; i < count; i++) {
        auto it = storage->db.find(keys[i]);
        if(it != storage->db.end()){
            exists++;
        }
    }

    snprintf(buf,sizeof(buf), ":%d\r\n", exists);
    send(fd_dest, buf, strlen(buf), 0);
    vars::MessageFactory::debug("EXISTS Success");
    return;
};

void GeneralCommand::MOVE(int fd_dest, const char* key, const char* _dest, const char* opt, const int duration, struct storage_holder* storage){
    char buf[RESPONSE_BUFFER];
    int optc = 0;

    while(opt != NULL) {
        optc++;
    }

    storage_type dest_db = RESP_Parser::parse_storage(_dest);
    auto prev_db = vars::MemoryStorage::find_key(key, storage);
    if(prev_db == NULL) {
        send(fd_dest, "$-1\r\n", 20, 0);
        return;
    }

    auto prev_it = prev_db->find(key);

    switch(dest_db) {
        case STANDARD: {
            if(prev_it->second.type == dest_db) {
                send(fd_dest, "-ERR key already exist\r\n", 24, 0);
                return;
            }

            struct StoreValue sv;
            sv.value = prev_it->second.value;
            sv.has_expiry = false;
            sv.type = STANDARD;
            storage->db[key] = sv;
            break;
        }
        case TEMPORARY: {
            if(prev_it->second.type == dest_db) {
                send(fd_dest, "-ERR key already exist\r\n", 24, 0);
                return;
            }

            if(optc < 1) {
                send(fd_dest, "-ERR insufficient argument\r\n", 50, 0);
                return;
            };

            int duration = atoi(opt);

            if(strncmp(opt, "PX", 2) == 0) {
                storage->db[key] = vars::MemoryStorage::make_expiry_value(prev_it->second.value.c_str(), std::chrono::milliseconds(duration));
            } 
            else if(strncmp(opt, "EX", 2) == 0) {
                storage->db[key] = vars::MemoryStorage::make_expiry_value(prev_it->second.value.c_str(), std::chrono::seconds(duration));
            }
            break;
        }
        case HASH: {
            break;
        }
        case LIST: {
            break;
        }
        case UNKNOWN: {
            snprintf(buf, sizeof(buf), "-ERR unknown storage destination: %s\r\n", _dest);
            send(fd_dest, buf, strlen(buf), 0);
            break;
        }
    }
    snprintf(buf, sizeof(buf), "+OK\r\n");
    send(fd_dest, buf, strlen(buf), 0);
    vars::MessageFactory::debug("MOVE Success");
    return;
};