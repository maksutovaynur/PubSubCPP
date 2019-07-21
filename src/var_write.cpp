//
// Created by aynur on 21.07.19.
//

#include "../lib/topic.hpp"

int main(){
    Variable::Ptr var = Variable::open_create("/my_variable1", sizeof(int));
    if (nullptr == var){
        std::cout << "Cannot create or open variable" << std::endl;
        return 1;
    }
    int data;
    std::cout << "Введите число: ";
    std::cin >> data;
    var->write(&data);
    return 0;
}