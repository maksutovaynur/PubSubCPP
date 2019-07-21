#include <iostream>
#include "../lib/topic.hpp"
#include <cstdio>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];

int main() {
    auto t = Topic::spawn_create("/mytopic_name", MSG_SIZE, 1);
    std::cout << "Started topic: " << (bool)(t != nullptr) << std::endl;
    if (t == nullptr) return 0;
    while (!Topic::was_interrupted()) {
        t->sub(msg);
        std::cout << msg << "|end" << std::endl;
    }
    return 0;
}
