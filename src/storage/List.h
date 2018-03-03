#ifndef AFINA_STORAGE_LIST_H
#define AFINA_STORAGE_LIST_H

#include <string>

class List{
    public:
        struct Entry;
    
        List() : _tail(nullptr), _head(nullptr) {};
        ~List();
        
        void PushFront(const std::string &key, const std::string &value);    
        void Del(Entry* node);    
        void Remove();
    
        std::string& GetValue(Entry* node);
        const std::string& GetTailKey() const;
        const std::string& GetHeadKey() const;
        Entry* Head() const;
    
        struct Entry{
            Entry() : _prev(nullptr), _next(nullptr), _key(nullptr) {};
            Entry(const std::string &key, const std::string &value, Entry* next, Entry* prev) : 
                                        _key(key), _value(value), _next(next), _prev(prev) {};
            ~Entry();
            size_t size(); 
            
            Entry* _prev;
            Entry* _next;
            std::string _value;
            const std::string _key;
        };      
    
    private:
        Entry* _tail;
        Entry* _head;
};

#endif
