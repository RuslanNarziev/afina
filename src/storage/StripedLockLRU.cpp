#include "StripedLockLRU.h"

namespace Afina {
namespace Backend {

StripedLockLRU::StripedLockLRU(std::size_t max_size, std::size_t n_shards) : shards_count(n_shards) {
    std::size_t one_storage_size = max_size / n_shards;
    if (one_storage_size < 1024 * 1024) {
        throw std::runtime_error("STORAGE ERROR: too small shard");
    }
    for (std::size_t i = 0; i < n_shards; ++i){
        shards.emplace_back(new ThreadSafeSimplLRU(one_storage_size));
    }
}

// See MapBasedGlobalLockImpl.h
bool StripedLockLRU::Put(const std::string &key, const std::string &value) { 
    return shards[hash(key) % shards_count]->Put(key, value);
}

// See MapBasedGlobalLockImpl.h
bool StripedLockLRU::PutIfAbsent(const std::string &key, const std::string &value) { 
    return shards[hash(key) % shards_count]->PutIfAbsent(key, value);
}

// See MapBasedGlobalLockImpl.h
bool StripedLockLRU::Set(const std::string &key, const std::string &value) { 
    return shards[hash(key) % shards_count]->Set(key, value);
}

// See MapBasedGlobalLockImpl.h
bool StripedLockLRU::Delete(const std::string &key) {
    return shards[hash(key) % shards_count]->Delete(key);
}

// See MapBasedGlobalLockImpl.h
bool StripedLockLRU::Get(const std::string &key, std::string &value) { 
    return shards[hash(key) % shards_count]->Get(key, value);
}

} // namespace Backend
} // namespace Afina