#include <iostream>

template <class T>
struct Node{
    Node() : prev(nullptr), next(nullptr) {};
    Node* prev;
    Node* next;
    T value;
};