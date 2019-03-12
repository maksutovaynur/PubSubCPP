#include <iostream>
#include "debug.hpp"
#define DEBUG DF4 | DF6
#include "topic.hpp"
#include <stdio.h>

template <typename T>
struct Question{
    T x = 0;
    T y = 0;
};


int main() {
    std::string service_name = "/clap0";
    std::cout << "Service : " << service_name << std::endl << "Enter 0 to serve, 1 to ask" << std::endl;

    int i;
    std::cin >> i;

    if (i == 0){
        auto s = Service<Question<int>, int>::create_server(service_name, 10);
        std::cout << " Starting serve" << std::endl;
        if (nullptr == s){
            std::cout << "Server wasnt created" << std::endl;
        }
        while (!tpc::interrupted){
            auto q = s->wait();
            if (nullptr == q) continue;
            auto msg = q->message();
            auto result = msg.x + msg.y;
            q->answer(result);
            std::cout << "SERVICE: [" << msg.x << ", " << msg.y << "] -> " << result << std::endl;
        }
    }else{
        auto s = Service<Question<int>, int>::create_client(service_name, 10);
        Question<int> msg;
        while(!tpc::interrupted){
            std::cin >> msg.x >> msg.y;
            std::cout << "Request: " << msg.x << " " << msg.y << std::endl;
            auto result = s->ask(msg, true);
            if (nullptr != result)
                std::cout << "Response: " << *result << std::endl;
            else std::cout << "Response: NULL" << std::endl;
        }
    }
    return 0;
}