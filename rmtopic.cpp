#include <iostream>
#include "topic.hpp"

int main(int argc, char** args) {
    std::string name = "/mytopic_name";
    if (Topic::remove(name)) std::cout << "Removed topic " << name << std::endl;
    return 0;
}