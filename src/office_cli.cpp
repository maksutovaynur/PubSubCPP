#include "../lib/topic.hpp"
#include <iostream>
#include <limits>

struct Question{
    int a;
    int b;
};

int main(){
    Office::Ptr off = Office::open_create("/my_funny_office1", sizeof(Question), sizeof(int)); // create new box or open if exists
    if (nullptr == off){
        std::cout << "Cannot create box!" << std::endl;
        return 1;
    }
    Question msg;
    int result;
    while (!tpc::interrupted) {
        std::cout << "x and y > ";
        std::cin >> msg.a >> msg.b;
        if (std::cin.fail()) {
            DEBUG_MSG("Fail while reading !", DF6);
            std::cin.clear();
            if (tpc::interrupted) break;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        off->ask(&msg);
        off->get_answer(&result);
        std::cout << result << std::endl;
    }
    std::cout << "End of office client work" << std::endl;
    return 0;
}