#include "../lib/topic.hpp"


int main(){
    Box::Ptr box = Box::open_create("/my_funny_box1", sizeof(int));  // Open existing box
    if (nullptr == box){
        std::cout << "Cannot open box!" << std::endl;
        return 1;
    }
    int data;
    std::cout << "Введите число" << std::endl;
    std::cin >> data;
    box->put(&data);
    return 0;
}