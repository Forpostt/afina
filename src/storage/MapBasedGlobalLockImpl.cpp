#include "MapBasedGlobalLockImpl.h"

#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) { 
    std::lock_guard<std::mutex> lock(_mutex);
    if (key.size() + value.size() > _max_size)
        return false;
    
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end()){
        while (key.size() + value.size() + _size > _max_size)
            DeleteUnlock(_list.GetTailKey());
    
        _list.PushFront(key, value);
        _hash_table.insert(std::make_pair(key, _list.Head()));
        _list.SetKey(_list.Head(), _hash_table.find(key)->first);
        _size += key.size() + value.size();
        return true; 
    }
    else{
        _list.GetValue(obj->second);
        while (key.size() + value.size() + _size - obj->second->size() > _max_size)
            DeleteUnlock(_list.GetTailKey());
    
        _size -= obj->second->size();
        _list.GetValue(obj->second) = value;
        _size += obj->second->size();
        return true; 
    }
    
    return false;
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) { 
    std::lock_guard<std::mutex> lock(_mutex);
    if (key.size() + value.size() > _max_size)
        return false;
        
    if (_hash_table.find(key) != _hash_table.end()){
        return false;
    }
    
    while (key.size() + value.size() + _size > _max_size)
            DeleteUnlock(_list.GetTailKey());
    
    _list.PushFront(key, value);
    _hash_table.insert(std::make_pair(key, _list.Head()));
    _list.SetKey(_list.Head(), _hash_table.find(key)->first);
    _size += key.size() + value.size();
    
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) { 
    std::lock_guard<std::mutex> lock(_mutex);
    if (key.size() + value.size() > _max_size)
        return false;
    
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    _list.GetValue(obj->second);
    while (key.size() + value.size() + _size - obj->second->size() > _max_size)
        DeleteUnlock(_list.GetTailKey());
    
    _size -= obj->second->size();
    _list.GetValue(obj->second) = value;
    _size += obj->second->size();
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) { 
    std::lock_guard<std::mutex> lock(_mutex);
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    _size -= obj->second->size();
    _list.Del(obj->second);
    _hash_table.erase(obj);

    return true; 
}

bool MapBasedGlobalLockImpl::DeleteUnlock(const std::string &key) { 
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    _size -= obj->second->size();
    _list.Del(obj->second);
    _hash_table.erase(obj);

    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const { 
    std::lock_guard<std::mutex> lock(_mutex);
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    value = _list.GetValue(obj->second);
    return true; 
}
} // namespace Backend
} // namespace Afina
