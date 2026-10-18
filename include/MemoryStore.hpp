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
} storage_type;

struct storage_list {
    const char* name;
    storage_type target;
};

extern struct storage_list storage_lists[];

struct StoreValue {
    storage_type type = UNKNOWN; 
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
    bool has_expiry = false;

    std::unordered_map<std::string, std::string>* fields;
    std::deque<std::string> items;
};

struct storage_holder {
    std::unordered_map<std::string, struct StoreValue> db;
};

constexpr storage_type cmp_temp() { return TEMPORARY; }
constexpr storage_type cmp_hash() { return HASH; }
constexpr storage_type cmp_list() { return LIST; }
constexpr storage_type cmp_standard() { return STANDARD; }

namespace vars {
    namespace MemoryStorage {
        struct ::storage_holder* allocate();
        void deallocate(struct ::storage_holder* target);
        bool is_expired(const struct ::StoreValue& sv);

        std::unordered_map<std::string, struct ::StoreValue>* find_key(const char* key, struct ::storage_holder* db);

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