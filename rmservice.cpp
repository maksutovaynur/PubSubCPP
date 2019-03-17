#include <iostream>
#include "topic.hpp"
#include <cstdio>

int main() {
    std::string service_name = "/clap0";
    if (service::remove(service_name)) {
        std::cout << "Service : " << service_name << " removed" << std::endl;
    }
    return 0;
}