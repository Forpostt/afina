template <class T>
struct Entry{
    Entry() : _prev(nullptr), _next(nullptr) {};
    Entry* _prev;
    Entry* _next;
    T value;
};

template <class T>
class List{
    public:
        List() : _tail(nullptr), _head(nullptr) {};
        ~List();
        bool push_front(T&& value);
        T& get(Entry<T>* node);
        bool del(Entry<T>* node);
        
        Entry<T>* _tail;
        Entry<T>* _head;
};

template <class T>
List<T>::~List(){
    Entry<T>* cur = _head;
    while (cur != nullptr){
        cur = _head->_next;
        _head->_next = nullptr;
        delete _head;
        _head = cur;
    }
}

template <class T>
bool List<T>::push_front(T&& value){
    if (_head == nullptr){
        Entry<T> *node = new Entry<T>;
        node->value = value;
        _head = node;
        _tail = node;
        return true;
    }
    Entry<T>* node = new Entry<T>;
    node->value = value;
    node->_next = _head;
    _head->_prev = node;
    _head = node;
    return true;
}

template <class T>
T& List<T>::get(Entry<T>* node){
    if (node == _head)
        return _head->value;
    
    if (node == _tail){
        _tail = node->_prev;
        _tail->_next = nullptr;
        node->_prev = nullptr;
        node->_next = _head;
        _head->_prev = node;
        _head = node;
        return _head->value;
    }
    
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_prev = nullptr;
    node->_next = _head;
    _head->_prev = node;
    _head = node;
    return _head->value;
}

template <class T>
bool List<T>::del(Entry<T>* node){
    if (node == _head){
        _head = node->_next;
        node->_next = nullptr;
        delete node;
        return true;
    }
    
    if (node == _tail){
        _tail = node->_prev;
        node->_prev = nullptr;
        delete node;
        return true;
    }
    
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    node->_prev = node->_next = nullptr;
    delete node;
    return true;
}
