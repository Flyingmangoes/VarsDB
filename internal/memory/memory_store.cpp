#include "MemoryStore.hpp"

struct ::storage_holder* vars::MemoryStorage::allocate() { 
    vars::MessageFactory::info("Allocating memory for storage");
    return new storage_holder(); // initilize storage
};

void vars::MemoryStorage::deallocate(struct ::storage_holder* target) {
    delete target; //remove storage instance
};

bool vars::MemoryStorage::is_expired(const ::StoreValue& sv) {
    if (!sv.has_expiry) return false;
    return std::chrono::steady_clock::now() > sv.expires_at;
}

std::unordered_map<std::string, struct ::StoreValue>* vars::MemoryStorage::find_key(const char* key, struct ::storage_holder* storage) {
    std::unordered_map<std::string, struct ::StoreValue>* result;

    auto it = storage->db.find(key);
    if(it == storage->db.end()) {  
        vars::MessageFactory::debug("key didn't exist");
        return NULL;
    }

    result->insert({it->first, it->second});
    return result;
};
