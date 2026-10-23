#include "Command.hpp"
#include "RESP.hpp"
#include <algorithm>
#include <cctype>
#include <cerrno>

namespace {
bool reply(int fd, const std::string& s) {
    size_t sent = 0;
    while (sent < s.size()) {
        ssize_t n = send(fd, s.data() + sent, s.size() - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}
}

GeneralCommand::GeneralCommand(Storage* storage) {
    this->storage = storage;
}

void GeneralCommand::ECHO(int fd_dest, const std::string& message) {
    reply(fd_dest, RESP_Parser::bulk_string(message));
    vars::MessageFactory::debug("ECHO Success");
}

void GeneralCommand::PING(int fd_dest) {
    reply(fd_dest, RESP_Parser::simple_string("PONG"));
    vars::MessageFactory::debug("PING Success");
}

void GeneralCommand::SET(int fd_dest, const std::string& key, 
    const std::string& value, const std::string& opt, int n) {
    if (!opt.empty()) {
        if (n <= 0) {
            reply(fd_dest, RESP_Parser::error("ERR invalid expire time"));
            return;
        }
        std::string o = upper(opt);
        if (o == "EX") {
            this->storage->db[key] = vars::MemoryStorage::make_expiry_value(value, std::chrono::seconds(n));
        } else if (o == "PX") {
            this->storage->db[key] = vars::MemoryStorage::make_expiry_value(value, std::chrono::milliseconds(n));
        } else {
            reply(fd_dest, RESP_Parser::error("ERR unknown option"));
            return;
        }
    } else {
        StoreValue sv;
        sv.value      = value;
        sv.has_expiry = false;
        sv.type       = STANDARD;
        this->storage->db[key] = sv;
    }

    reply(fd_dest, RESP_Parser::simple_string("OK"));
    vars::MessageFactory::debug("SET Success");
}

void GeneralCommand::GET(int fd_dest, const std::string& key) {
    auto it = this->storage->db.find(key);
    if (it == this->storage->db.end()) {
        reply(fd_dest, RESP_Parser::null_bulk());
        return;
    }

    if (it->second.has_expiry && vars::MemoryStorage::is_expired(it->second)) {
        this->storage->db.erase(it);
        reply(fd_dest, RESP_Parser::null_bulk());
        return;
    }

    reply(fd_dest, RESP_Parser::bulk_string(it->second.value));
    vars::MessageFactory::debug("GET Success");
}

void GeneralCommand::DEL(int fd_dest, const std::vector<std::string>& keys) {
    long long deleted = 0;
    for (const auto& k : keys) {
        deleted += this->storage->db.erase(k);
    }
    reply(fd_dest, RESP_Parser::integer(deleted));
}

void GeneralCommand::EXISTS(int fd_dest, const std::vector<std::string>& keys) {
    long long exists = 0;
    for (const auto& k : keys) {
        if (this->storage->db.find(k) != this->storage->db.end()) exists++;
    }
    reply(fd_dest, RESP_Parser::integer(exists));
    vars::MessageFactory::debug("EXISTS Success");
}

void GeneralCommand::MOVE(int fd_dest, const std::string& key, 
    const std::string& storage_dest, const std::string& opt, int duration) {
    StorageType dest_db = RESP_Parser::parse_storage(storage_dest);
    ::StoreValue* prev = vars::MemoryStorage::find_key(key, storage);
    if (prev == nullptr) {
        reply(fd_dest, RESP_Parser::null_bulk());
        return;
    }

    
    const std::string prev_value = prev->value;
    const StorageType prev_type  = prev->type;

    switch (dest_db) {
        case STANDARD: {
            if (prev_type == dest_db) {
                reply(fd_dest, RESP_Parser::error("ERR key already exist"));
                return;
            }
            StoreValue sv;
            sv.value      = prev_value;
            sv.has_expiry = false;
            sv.type       = STANDARD;
            this->storage->db[key] = sv;
            break;
        }
        case TEMPORARY: {
            if (prev_type == dest_db) {
                reply(fd_dest, RESP_Parser::error("ERR key already exists"));
                return;
            }
            if (opt.empty() || duration <= 0) {
                reply(fd_dest, RESP_Parser::error("ERR insufficient argument"));
                return;
            }
            std::string o = upper(opt);
            if (o == "PX") {
                this->storage->db[key] = vars::MemoryStorage::make_expiry_value(prev_value, std::chrono::milliseconds(duration));
            } else if (o == "EX") {
                this->storage->db[key] = vars::MemoryStorage::make_expiry_value(prev_value, std::chrono::seconds(duration));
            } else {
                reply(fd_dest, RESP_Parser::error("ERR unknown option"));
                return;
            }
            break;
        }
        case HASH:
        case LIST:
            reply(fd_dest, RESP_Parser::error("ERR destination not implemented"));
            return;
        case UNKNOWN:
        default:
            reply(fd_dest, RESP_Parser::error("ERR unknown destination"));
            return;
    }

    reply(fd_dest, RESP_Parser::simple_string("OK"));
    vars::MessageFactory::debug("MOVE Success");
}