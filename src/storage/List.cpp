#include "List.h"

size_t List::Entry::size(){
    return _key->size() + _value.size();
}

List::~List(){
    Remove();
}

List::Entry* List::Head() const{
    return _head;
}

List::Entry::~Entry(){
    _value.clear();
    _key = nullptr;
}

void List::PushFront(const std::string &key, const std::string &value){
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
    return *(_tail->_key);
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

void List::Remove(){
    Entry* cur = _head;
    while (cur != nullptr){
        cur = _head->_next;
        _head->_next = nullptr;
        delete _head;
        _head = cur;
    }
}

void List::SetKey(Entry* node, const std::string& key){
    node->_key = &key;
}