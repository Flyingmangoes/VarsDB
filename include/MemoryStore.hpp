#ifndef MEMORY_STORE_H
#define MEMORY_STORE_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <deque>
#include "Message.hpp"

typedef enum {
    TEMPORARY = 1,
    STANDARD = 2,
    HASH = 3,
    LIST = 4,
    UNKNOWN = 0
} StorageType;

struct StorageList {
    const char* name;
    StorageType target;
};

extern struct StorageList storage_lists[];

struct StoreValue {
    StorageType type = UNKNOWN; 
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
    bool has_expiry = false;

    std::unordered_map<std::string, std::string>* fields;
    std::deque<std::string> items;
};

struct Storage {
    std::unordered_map<std::string, struct StoreValue> db;
};

const StorageType cmp_temp() { return TEMPORARY; }
const StorageType cmp_hash() { return HASH; }
const StorageType cmp_list() { return LIST; }
const StorageType cmp_standard() { return STANDARD; }

namespace vars {
    namespace MemoryStorage {
        struct ::Storage* allocate();
        void deallocate(struct ::Storage* target);
        bool is_expired(const struct ::StoreValue& sv);

        std::unordered_map<std::string, struct ::StoreValue>* find_key(const char* key, struct ::Storage* db);
        template<typename Duration> struct ::StoreValue make_expiry_value(const char* value, Duration duration) {
            StoreValue sv;
            sv.value      = std::string(value);
            sv.has_expiry = true;
            sv.expires_at = std::chrono::steady_clock::now() + duration;
            sv.type = TEMPORARY;
            return sv;
        };
    };
};

#endif