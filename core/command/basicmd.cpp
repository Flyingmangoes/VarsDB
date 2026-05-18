#include "utilities.hpp"
#include "command.hpp"

void GeneralCommand::ECHO(int fd_dest, const char* message) {
    char echo_response[MAX_RESPONSE_BUFFER];
    snprintf(echo_response, sizeof(echo_response),"+%s\r\n", message);
    send(fd_dest, echo_response, strlen(echo_response), 0);
    return;
};

void GeneralCommand::PING(int fd_dest) {
    const char response[MAX_RESPONSE_BUFFER] = "+PONG\r\n";
    send(fd_dest, response, strlen(response), 0);
    return;
};

void GeneralCommand::SET(int fd_dest, const char* key, const char* value, const char* opt, int n, struct storage_holder* storage) {
    if(opt != NULL && n > 0) {
        if(strncmp(opt, "EX", 2) == 0) {
            storage->db[key] = strg::make_expiry_value(value, std::chrono::seconds(n));
        }
        else if(strncmp(opt, "PX", 2) == 0) {
            storage->db[key] = strg::make_expiry_value(value, std::chrono::milliseconds(n));
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
        sv.type = sv.STANDARD;
        storage->db[key]     = sv;
    }

    send(fd_dest, "+OK\r\n", 5, 0);
    return;
};

void GeneralCommand::GET(int fd_dest, const char* key, struct storage_holder* storage) {
    char buf[MAX_RESPONSE_BUFFER];

    auto it = storage->db.find(key);
    if(it == storage->db.end()) {
        send(fd_dest, "$-1\r\n", 5, 0);
        return;
    }

    if(it->second.type == StoreValue::TEMPORARY){
        if(strg::is_expired(it->second)) {
            storage->db.erase(it);
            send(fd_dest, "$-1\r\n", 5, 0);
            return;
        }
    }

    snprintf(buf, sizeof(buf), "+%s\r\n", it->second.value.c_str());
    send(fd_dest, buf, strlen(buf), 0);
    return;
};

void GeneralCommand::DEL(int fd_dest, int count, const char** keys, struct storage_holder* storage) {
    char response[MAX_RESPONSE_BUFFER];
    int deleted = 0;

    for (int i = 0; i < count; i++) {
        deleted += storage->db.erase(keys[i]);
    }

    snprintf(response, sizeof(response), ":%d\r\n", deleted);
    send(fd_dest, response, strlen(response), 0);
};

void GeneralCommand::EXISTS(int fd_dest, int count, const char** keys, struct storage_holder* storage) {
    char buf[MAX_RESPONSE_BUFFER];
    int exists = 0;

    for (int i = 0; i < count; i++) {
        auto it = storage->db.find(keys[i]);
        if(it != storage->db.end()){
            exists++;
        }
    }

    snprintf(buf,sizeof(buf), ":%d\r\n", exists);
    send(fd_dest, buf, strlen(buf), 0);
};

void GeneralCommand::MOVE(int fd_dest, const char* key, const char* _dest, struct storage_holder* storage){
    char buf[MAX_RESPONSE_BUFFER];

    storage_target dest = utils::Parser::parse_storage(_dest);
    auto prev = strg::find_key(key, storage);

    switch(dest) {
        case DEST_STANDARD: {
            break;   
        }
        case DEST_TEMPORARY: {
            break;
        }
        case DEST_HASH: {
            break;
        }
        case DEST_LIST: {
            break;
        }
        case DEST_UNKNOWN: 
        default: {
            snprintf(buf, sizeof(buf), "-ERR unknown storage destination: %s\r\n", _dest);
            send(fd_dest, buf, strlen(buf), 0);
            break;
        }
    }
};