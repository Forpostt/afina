#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) { 
    if (_hash_table.find(key) != _hash_table.end())
        return false;
        
    _list.push_front(value);
    _hash_table.insert(std::make_pair(key, _list.head));
    
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) { 
    if (_hash_table.find(key) != _hash_table.end()){
        return false;
    }
    _list.push_front(value);
    _hash_table.insert(std::make_pair(key, _list.head));
    
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) { 
    auto it = _hash_table.find(key);
    if (it == _hash_table.end())
        return false;
    
    _list.get(it->second) = value;
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) { 
    auto it = _hash_table.find(key);
    if (it == _hash_table.end())
        return false;
    
    _list.del(it->second);
    _hash_table.erase(it);

    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const { 
    auto it = _hash_table.find(key);
    if (it == _hash_table.end())
        return false;
    
    value = _list.get(it->second);
    return true; 

    return false; 
}

} // namespace Backend
} // namespace Afina
