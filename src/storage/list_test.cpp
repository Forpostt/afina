#include "List.hpp"
#include <iostream>

void test_1(){
    std::cout << "test 1" << std::endl;
    List<int> my_list;
    my_list.push_front(10);
    my_list.push_front(11);
    std::cout << my_list.head->value << std::endl;
    std::cout << my_list.tail->value << std::endl;
    std::cout << my_list.head->next->value << std::endl;
    std::cout << my_list.tail->prev->value << std::endl;
}

void test_2(){
    std::cout << "test 2" << std::endl;
    List<int> my_list;
    my_list.push_front(10);
    std::cout << my_list.head->value << std::endl;
    std::cout << my_list.tail->value << std::endl;
}

void test_3(){
    std::cout << "test 3" << std::endl;
    List<int> my_list;
    my_list.push_front(10);
    my_list.push_front(11);
    my_list.push_front(12);
    Entry<int>* node = my_list.head->next;
    my_list.get(node);
    std::cout << my_list.head->value << std::endl;
    std::cout << my_list.head->next->value << std::endl;
    std::cout << my_list.tail->value << std::endl;
    std::cout << std::endl;
    my_list.get(my_list.tail);
    std::cout << my_list.head->value << std::endl;
    std::cout << my_list.head->next->value << std::endl;
    std::cout << my_list.tail->value << std::endl;
}

int main(){
    test_1();
    test_2();
    test_3();
    return 0;   
}