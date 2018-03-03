#include "MapBasedGlobalLockImpl.h"

#include <iostream>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) { 
    std::lock_guard<std::mutex> lock(_mutex);
    if (key.size() + value.size() > _max_size)
        return false;
    
    std::reference_wrapper<const std::string> key_ref(key);
    auto obj = _hash_table.find(key_ref);
    if (obj == _hash_table.end()){
        while (key.size() + value.size() + _size > _max_size)
            DeleteUnlock(_list.GetTailKey());
    
        _list.PushFront(key, value);
        _hash_table.insert(std::make_pair (std::reference_wrapper<const std::string> (_list.GetHeadKey()), _list.Head()));
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
    
    std::reference_wrapper<const std::string> key_ref(key);
    if (_hash_table.find(key_ref) != _hash_table.end()){
        return false;
    }
    
    while (key.size() + value.size() + _size > _max_size)
            DeleteUnlock(_list.GetTailKey());
    
    _list.PushFront(key, value);
    _hash_table.insert(std::make_pair (std::reference_wrapper<const std::string> (_list.GetHeadKey()), _list.Head()));
    _size += key.size() + value.size();
    
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) { 
    std::lock_guard<std::mutex> lock(_mutex);
    if (key.size() + value.size() > _max_size)
        return false;
    
    std::reference_wrapper<const std::string> key_ref(key);
    auto obj = _hash_table.find(key_ref);
    if (obj == _hash_table.end())
        return false;
    
    _list.GetValue(obj->second);
    while (key.size() + value.size() + _size - obj->second->size() > _max_size)
        DeleteUnlock(_list.GetTailKey());
    //rehash?
    _size -= obj->second->size();
    _list.GetValue(obj->second) = value;
    _size += obj->second->size();
    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) { 
    std::lock_guard<std::mutex> lock(_mutex);
    std::reference_wrapper<const std::string> key_ref(key);
    auto obj = _hash_table.find(key_ref);
    if (obj == _hash_table.end())
        return false;
    
    auto node = obj->second;
    _size -= node->size();
    _hash_table.erase(obj);
    _list.Del(node);

    return true; 
}

bool MapBasedGlobalLockImpl::DeleteUnlock(const std::string &key) { 
    std::reference_wrapper<const std::string> key_ref(key);
    auto obj = _hash_table.find(key);
    if (obj == _hash_table.end())
        return false;
    
    auto node = obj->second;
    _size -= node->size();
    _hash_table.erase(obj);
    _list.Del(node);

    return true; 
}

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const { 
    std::lock_guard<std::mutex> lock(_mutex);
    std::reference_wrapper<const std::string> key_ref(key);
    auto obj = _hash_table.find(key_ref);
    if (obj == _hash_table.end())
        return false;
    
    value = _list.GetValue(obj->second);
    return true; 
}

} // namespace Backend
} // namespace Afina
