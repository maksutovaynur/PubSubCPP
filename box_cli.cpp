#include "topic.hpp"
#include <iostream>

int main(){
    Box::Ptr box = Box::open_create("/my_funny_box1", sizeof(int)); // create new box or open if exists
    if (nullptr == box){
        std::cout << "Cannot create box!" << std::endl;
        return 1;
    }
    int data;
    box->get(&data);                 // Await when data appears in box. Blocks thread
    std::cout << data << std::endl;
    return 0;
}