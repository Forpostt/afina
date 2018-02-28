#ifndef AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H
#define AFINA_STORAGE_MAP_BASED_GLOBAL_LOCK_IMPL_H

#include <string>
#include <iostream>

class List{
    public:
        struct Entry;
    
        List() : _tail(nullptr), _head(nullptr) {};
        ~List();
        
        void PushFront(std::string &key, std::string &value);
        void ToFront(Entry* node);    
        void Del(Entry* node);    
        std::string& GetValue(Entry* node);
        const std::string& GetTailKey() const;
    
        struct Entry{
            Entry() : _prev(nullptr), _next(nullptr) {};
            Entry(std::string &key, std::string &value, Entry* next, Entry* prev) : 
                                        _key(key), _value(value), _next(next), _prev(prev) {};
            size_t size(); 
            
            Entry* _prev;
            Entry* _next;
            std::string _value;
            const std::string _key;
        };      
    
    //private:
        Entry* _tail;
        Entry* _head;
};

size_t List::Entry::size(){
    return _key.size() + _value.size();
}

List::~List(){
    std::cout << "~List()" << std::endl;
    Entry* cur = _head;
    while (cur != nullptr){
        std::cout << _head->_value << std::endl;
        cur = _head->_next;
        _head->_next = nullptr;
        delete _head;
        _head = cur;
    }
}

void List::PushFront(std::string &key, std::string &value){
    if (_head == nullptr){
        Entry* node = new Entry(key, value, nullptr, nullptr);
        _head = node;
        _tail = node;
        return;
    }
    Entry* node = new Entry(key, value, _head, nullptr);
    _head->_prev = node;
    _head = node;
    return;
}

void List::ToFront(Entry* node){
    if (_head == node)
        return;
    
    if (_tail == node){
        _tail = node->_prev;
        _tail->_next = nullptr;
        node->_prev = nullptr;
        node->_next = _head;
        _head->_prev = node;
        _head = node;
        return;
    }
    
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_prev = nullptr;
    node->_next = _head;
    _head->_prev = node;
    _head = node;
    return;
}

std::string& List::GetValue(Entry* node){
    if (node == _head)
        return _head->_value;
    
    if (node == _tail){
        _tail = node->_prev;
        _tail->_next = nullptr;
        node->_prev = nullptr;
        node->_next = _head;
        _head->_prev = node;
        _head = node;
        return _head->_value;
    }
    
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_prev = nullptr;
    node->_next = _head;
    _head->_prev = node;
    _head = node;
    return _head->_value;
}

const std::string& List::GetTailKey() const{
    return _tail->_key;
}

void List::Del(Entry* node){
    if (_head == _tail){
        delete _head;
        _head = _tail = nullptr;
        return;
    }
    
    if (node == _head){
        _head->_next->_prev = nullptr;
        _head = node->_next;
        node->_next = nullptr;
        delete node;
        return;
    }
    
    if (node == _tail){
        _tail->_prev->_next = nullptr;
        _tail = node->_prev;
        node->_prev = nullptr;
        delete node;
        return;
    }
    
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_prev = node->_next = nullptr;
    delete node;
    return;
}

#endif