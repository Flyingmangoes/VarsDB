#ifndef STORE_H
#define STORE_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <deque>
#include <utilities.hpp>

struct StoreValue {
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
    bool has_expiry;

    std::unordered_map<std::string, std::string> fields; // hash
    std::deque<std::string> items; // list

    enum StorageType { STANDARD, TEMPORARY, HASH, LIST } type;
};

struct storage_holder {
    /* List for storing ordered lis
       Hash for storing a bunch of value inside a fields
       Standard is basic key value pair with single value
       Temporary store the key and value with a duration 
    */
    std::unordered_map<std::string, struct StoreValue> db;
};

namespace strg {
    struct ::storage_holder* allocator();
    void deallocator(struct ::storage_holder* target);
    bool is_expired(const struct ::StoreValue& sv);

    std::unordered_map<std::string, struct ::StoreValue>* find_key(const char* key, struct ::storage_holder* db);

    template<typename Duration> struct ::StoreValue make_expiry_value(const char* value, Duration duration) {
        StoreValue sv;
        sv.value      = std::string(value);
        sv.has_expiry = true;
        sv.expires_at = std::chrono::steady_clock::now() + duration;
        sv.type = sv.TEMPORARY;
        return sv;
    }
};

#endif