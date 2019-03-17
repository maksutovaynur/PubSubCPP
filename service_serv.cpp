#include <iostream>
#include "topic.hpp"
#include <cstdio>

template<typename T>
struct Question {
    T x = 0;
    T y = 0;
};


int main() {
    std::string service_name = "/clap0";
    auto s = service::create_async_server<8, 4>(service_name, 10, 10);
    if (nullptr == s) {
        std::cout << "Server wasnt created" << std::endl;
    }
    std::cout << " Starting serve" << std::endl;
    while (!tpc::interrupted) {
        auto q = s->wait_request();
        if (nullptr == q) {
            DEBUG_MSG("Query is null_ptr", DF6);
            continue;
        }
        if (sizeof(Question<int>) < q->size()) {
            DEBUG_MSG("Query data size "
            + std::to_string(q->size())
            + " if bigger than expected - "
            + std::to_string(sizeof(Question<int>)), DF6);
            continue;
        }
        auto msg = (Question<int>*) q->data();
        auto result = msg->x + msg->y;

        std::cout << "SERVICE: [" << msg->x << ", " << msg->y << "]";
        if (q->requires_answer()) {
            std::cout << " -> " << result;
            q->answer(&result, sizeof(int));
        }
        std::cout << std::endl;
    }

    return 0;
}