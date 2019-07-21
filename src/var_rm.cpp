//
// Created by aynur on 21.07.19.
//

#include "../lib/topic.hpp"

int main(){
    auto t = Variable::remove("/my_variable1");
    if (t) std::cout << "/my_variable1 rm: ok (" << t << ")" << std::endl;
    else  std::cout << "/my_variable1 rm: fail (" << t << ")" << std::endl;
    return 0;
}