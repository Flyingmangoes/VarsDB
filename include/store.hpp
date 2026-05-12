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
};

struct HashValue {
    std::unordered_map<std::string, std::string> fields;
};

struct ListValue {
    std::deque<std::string> items;
};

struct storage_holder {
    /* List for storing ordered lis
       Hash for storing a bunch of value inside a fields
       Standard is basic key value pair with single value
       Temporary store the key and value with a duration 
    */
    std::unordered_map<std::string, struct ListValue> List;
    std::unordered_map<std::string, struct HashValue> Hash;
    std::unordered_map<std::string, struct StoreValue> Standard;
    std::unordered_map<std::string, struct StoreValue> Temporary;
};

struct storage_holder* _storage_allocator();
void _deallocator(struct storage_holder* target);
bool is_expired(const StoreValue& sv);

template<typename Duration>
inline StoreValue make_expiry_value(const char* value, Duration duration) {
    StoreValue sv;
    sv.value      = std::string(value);
    sv.has_expiry = true;
    sv.expires_at = std::chrono::steady_clock::now() + duration;
    return sv;
}

#endif