#include <iostream>
#include "topic.hpp"
#include <cstdio>


int main(int argc, char** args) {
    std::string name = "/clap0";
    if (argc > 1) name = std::string(args[1]);
    std::cout << Topic::UI_SZ << "=ui|hdr=" << Topic::HDR_SZ << std::endl;
    if (Topic::remove(name)) std::cout << "Removed topic " << name << std::endl;
    return 0;
}