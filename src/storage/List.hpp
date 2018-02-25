#include "Node.hpp"

template <class T>
class List{
    public:
        List() : tail(nullptr), head(nullptr) {};
        ~List();
        void push_front(T&& value);
        T& get(Node<T>* node);
        void del(Node<T>* node);
        Node<T>* tail;
        Node<T>* head;
};

template <class T>
List<T>::~List(){
    Node<T>* cur = head;
    while (cur != nullptr){
        cur = head->next;
        head->next = nullptr;
        delete head;
        head = cur;
    }
}

template <class T>
void List<T>::push_front(T&& value){
    if (head == nullptr){
        Node<T> *node = new Node<T>;
        node->value = value;
        head = node;
        tail = node;
        return;
    }
    Node<T>* node = new Node<T>;
    node->value = value;
    node->next = head;
    head->prev = node;
    head = node;
    return;
}

template <class T>
T& List<T>::get(Node<T>* node){
    if (node == head)
        return head->value;
    
    if (node == tail){
        tail = node->prev;
        tail->next = nullptr;
        node->prev = nullptr;
        node->next = head;
        head->prev = node;
        head = node;
        return head->value;
    }
    
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = head;
    head->prev = node;
    head = node;
    return head->value;
}

template <class T>
void List<T>::del(Node<T>* node){
    if (node == head){
        head = node->next;
        node->next = nullptr;
        delete node;
        return;
    }
    
    if (node == tail){
        tail = node->prev;
        node->prev = nullptr;
        delete node;
        return;
    }
    
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node->next = nullptr;
    delete node;
    return;
}