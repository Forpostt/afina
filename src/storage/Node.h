#include <iostream>

template <class T>
struct Node{
    Node() : prev(nulltr), next(nullptr) {};
    Node* prev;
    Node* next;
    T value;
}