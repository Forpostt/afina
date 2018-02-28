#include "List.hpp"
#include <iostream>
#include <string>

void test_2(){
    std::cout << "test 2" << std::endl;
    List my_list;
    std::string s;
    s = "asd";
    my_list.PushFront(s, s);
    s = "qwe";
    my_list.PushFront(s, s);
    s = "vcb";
    my_list.PushFront(s, s);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._head->_next->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
}

void test_1(){
    std::cout << "test 1" << std::endl;
    List my_list;
    std::string s;
    s = "asd";
    my_list.PushFront(s, s);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << (my_list._tail == my_list._head) << std::endl;
}

void test_3(){
    std::cout << "test 3" << std::endl;
    List my_list;
    std::string s;
    s = "asd";
    my_list.PushFront(s, s);
    s = "qwe";
    my_list.PushFront(s, s);
    s = "vcb";
    my_list.PushFront(s, s);
    my_list.ToFront(my_list._tail);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._tail->_prev->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
    my_list.ToFront(my_list._tail->_prev);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._head->_next->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
}

void test_4(){
    std::cout << "test 4" << std::endl;
    List my_list;
    std::string s;
    s = "asd";
    my_list.PushFront(s, s);
    s = "qwe";
    my_list.PushFront(s, s);
    s = "vcb";
    my_list.PushFront(s, s);
    s = "tyu";
    my_list.PushFront(s, s);
    my_list.GetValue(my_list._tail->_prev);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._head->_next->_value << std::endl;
    std::cout << my_list._tail->_prev->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
}

void test_5(){
    std::cout << "test 5" << std::endl;
    List my_list;
    std::string s;
    s = "asd";
    my_list.PushFront(s, s);
    s = "qwe";
    my_list.PushFront(s, s);
    s = "vcb";
    my_list.PushFront(s, s);
    s = "tyu";
    my_list.PushFront(s, s);
    s = "iop";
    my_list.PushFront(s, s);
    my_list.Del(my_list._tail->_prev);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._head->_next->_value << std::endl;
    std::cout << my_list._tail->_prev->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
    my_list.Del(my_list._tail);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._head->_next->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
    my_list.Del(my_list._head);
    std::cout << my_list._head->_value << std::endl;
    std::cout << my_list._tail->_value << std::endl;
    std::cout << std::endl;
}

int main(){
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    return 0;   
}