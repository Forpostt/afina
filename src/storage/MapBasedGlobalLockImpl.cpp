#include "MapBasedGlobalLockImpl.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) { 
    if (key.size() + value.size() > _max_size)
        return false;
    
    obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return PutIfAbsent(key, value);
    else:
        return Set(key, value);
    
    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) { 
    if (key.size() + value.size() > _max_size)
        return false;
        
    if (_hash_table.find(key) != _hash_table.end()){
        return false;
    }
    
    if (key.size() + value.size() + _size > _max_size)
        return PutCrowdingOut(key, value);
    
    _list.PushFront(key, value);
    _hash_table.insert(std::make_pair(key, _list.head));
    _size += key.size + value.size();
    
    return true; 
}

bool MapBasedGlobalLockImpl::PutCrowdingOut(const std::string &key, const std::string &value) {
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end()){
        
        while (key.size() + value.size() + _size > _max_size)
            Delete(_list.GetTailKey());
        
        _list.PushFront(key, value);
        _size += key.size() + value.size();
        return true;
    }
    
    _list.ToFront(obj->second);
    
    while (key.size() + value.size() + _size - obj->second->size() > _max_size)
        Delete(_list.GetTailKey());
    
    _size -= obj->second->size();
    _list.GetValue(obj->second) = value;
    _size -= obj->second->size();
    return true;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) { 
    if (key.size() + value.size() > _max_size)
        return false;
    
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    if (key.size() + value.size() + _size - obj->second->size() > _max_size)
        return PutCrowdingOut(key, value);
    
    _size -= obj->second->size();
    _list.Get(obj->second) = value;
    _size += obj->second->size();
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) { 
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    _size -= obj->second->size();
    _list.del(obj->second);
    _hash_table.erase(obj);

    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const { 
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    value = _list.Get(obj->second);
    return true; 
}

} // namespace Backend
} // namespace Afina
