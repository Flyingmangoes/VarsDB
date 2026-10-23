#ifndef MEMORY_STORE_H
#define MEMORY_STORE_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <chrono>
#include <deque>
#include "Message.hpp"

enum StorageType {
    TEMPORARY = 1,
    STANDARD = 2,
    HASH = 3,
    LIST = 4,
    UNKNOWN = 0
};

struct StorageList {
    const char* name;
    StorageType target;
};

extern struct StorageList storage_list[];

struct StoreValue {
    StorageType type = UNKNOWN;
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
    bool has_expiry = false;

    std::unordered_map<std::string, std::string>* fields = nullptr; 
    std::deque<std::string> items;
};

struct Storage {
    std::unordered_map<std::string, StoreValue> db;
};

inline StorageType cmp_temp()     { return TEMPORARY; }  
inline StorageType cmp_hash()     { return HASH; }
inline StorageType cmp_list()     { return LIST; }
inline StorageType cmp_standard() { return STANDARD; }

namespace vars {
    namespace MemoryStorage {
        Storage* allocate();
        void deallocate(Storage* target);
        bool is_expired(const ::StoreValue& sv);

        ::StoreValue* find_key(std::string_view key, struct Storage* db);
        template<typename Duration> ::StoreValue make_expiry_value(std::string value, Duration duration) {
            StoreValue sv;
            sv.value      = value;
            sv.has_expiry = true;
            sv.expires_at = std::chrono::steady_clock::now() + duration;
            sv.type = TEMPORARY;
            return sv;
        };
    };
};

#endif