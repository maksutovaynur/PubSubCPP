#include <iostream>
#include "debug.hpp"

#define DEBUG  DF4 | DF6

#include "topic.hpp"
#include <stdio.h>

template<typename T>
struct Question {
    T x = 0;
    T y = 0;
};

int main() {
    std::string service_name = "/clap0";

    auto s = Service<Question<int>, int>::create_client(service_name, 10);
    Question<int> msg;
    while (!tpc::interrupted) {
        std::cin >> msg.x >> msg.y;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Request: " << msg.x << " " << msg.y << std::endl;
            continue;
        }
        auto result = s->sync_ask(msg, true);
        if (nullptr != result)
            std::cout << "Response: " << *result << std::endl;
        else std::cout << "Response: NULL" << std::endl;
    }

    return 0;
}