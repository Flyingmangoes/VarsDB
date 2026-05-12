#include <iostream>
#include "store.hpp"

struct storage_holder* _storage_allocator() { 
    MessageManager::info("Allocating memory for storage");
    return new struct storage_holder(); // initilize storage
};

void _deallocator(struct storage_holder* target) {
    delete target; //remove storage instance
};

bool is_expired(const StoreValue& sv) {
    if (!sv.has_expiry) return false;
    return std::chrono::steady_clock::now() > sv.expires_at;
}

