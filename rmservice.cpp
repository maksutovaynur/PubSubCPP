#include <iostream>
#include "topic.hpp"
#include <stdio.h>

template <typename T>
struct Question{
    T x = 0;
    T y = 0;
};


int main() {
    std::string service_name = "/clap0";
    std::cout << "Service : " << service_name << " removed" << std::endl;
    service::remove(service_name);
    return 0;
}