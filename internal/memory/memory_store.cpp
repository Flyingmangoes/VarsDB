#include "MemoryStore.hpp"

struct ::Storage* vars::MemoryStorage::allocate() { 
    vars::MessageFactory::info("Allocating memory for storage");
    return new Storage(); 
};

void vars::MemoryStorage::deallocate(::Storage* target) {
    delete target;
};

bool vars::MemoryStorage::is_expired(const ::StoreValue& sv) {
    if (!sv.has_expiry) return false;
    return std::chrono::steady_clock::now() > sv.expires_at;
}

::StoreValue* vars::MemoryStorage::find_key(std::string_view key, ::Storage* storage) {
    auto it = storage->db.find(std::string(key));
    if(it == storage->db.end()) {  
        vars::MessageFactory::debug("key didn't exist");
        return nullptr;
    }

    return &it->second;
};
