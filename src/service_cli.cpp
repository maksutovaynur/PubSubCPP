#include <iostream>
#include "lib/topic.hpp"
#include <cstdio>
#include <limits>

template<typename T>
struct Question {
    T x = 0;
    T y = 0;
};

int main() {
    std::string service_name = "/clap0";
    using Q = Question<int>;
    auto s = service::create_sync_client<sizeof(Q), sizeof(int)>(service_name);
    std::cout << "created service " << service_name << std::endl;
    Q msg;
    while (!tpc::interrupted) {
        std::cout << "x and y > ";
        std::cin >> msg.x >> msg.y;
        if (std::cin.fail()) {
            DEBUG_MSG("Fail while reading !", DF6);
            std::cin.clear();
            if (tpc::interrupted) break;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        int response;
        ui cnt = s->ask(&msg, &response);
        if (cnt > 0)
            std::cout << "Response: " << response
            << std::endl;
        else std::cout << "Response: NULL" << std::endl;
    }

    return 0;
}
