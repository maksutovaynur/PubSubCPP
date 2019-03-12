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

    auto s = Service<Question<int>, int>::create_server(service_name, 10);
    std::cout << " Starting serve" << std::endl;
    if (nullptr == s) {
        std::cout << "Server wasnt created" << std::endl;
    }
    while (!tpc::interrupted) {
        auto q = s->wait();
        if (nullptr == q) continue;
        auto msg = q->message();
        auto result = msg.x + msg.y;
        std::cout << "SERVICE: [" << msg.x << ", " << msg.y << "]";
        if (q->requires_answer()) {
            std::cout << " -> " << result;
            q->answer(result);
        }
        std::cout << std::endl;
    }

    return 0;
}