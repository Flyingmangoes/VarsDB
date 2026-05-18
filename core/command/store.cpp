#include "store.hpp"

struct ::storage_holder* strg::allocator() { 
    utils::MessageManager::info("Allocating memory for storage");
    return new storage_holder(); // initilize storage
};

void strg::deallocator(struct ::storage_holder* target) {
    delete target; //remove storage instance
};

bool strg::is_expired(const ::StoreValue& sv) {
    if (!sv.has_expiry) return false;
    return std::chrono::steady_clock::now() > sv.expires_at;
}

std::unordered_map<std::string, struct ::StoreValue>* strg::find_key(const char* key, struct ::storage_holder* storage) {

};
