#include <iostream>
#include "lib/debug.hpp"
//#define DEBUG DF6 | DF2 | DF4 | DF3 | DF5
#include "lib/topic.hpp"
#include <cstdio>

template<typename T>
struct Question {
    T x = 0;
    T y = 0;
};


int main() {
    std::string service_name = "/clap0";
    using Q=Question<int>;
    auto s = service::create_async_server<sizeof(Q), sizeof(int)>(service_name, 10, 10);
    if (nullptr == s) {
        std::cout << "Server wasnt created" << std::endl;
    }
    std::cout << " Starting serve" << std::endl;
    while (!tpc::interrupted) {
        DEBUG_MSG("Entered cycle", DF6);
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
