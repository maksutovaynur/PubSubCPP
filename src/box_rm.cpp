#include "../lib/topic.hpp"
#include <iostream>

int main(){
    auto t = Box::remove("/my_funny_box1"); // create new box or open if exists
    if (t) std::cout << "Ok: " << t << std::endl;
    else  std::cout << t << std::endl;
    return 0;
}