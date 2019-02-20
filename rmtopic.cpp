#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "topic.hpp"
#include <stdio.h>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];
volatile bool interrupted;

void signal_handler(int i){
    interrupted = true;
}

int main() {
    signal(SIGINT, signal_handler);
    auto t = topic::Topic::spawn("/clap0");
    std::cout << "Started topic: " << (bool)(t != nullptr) << std::endl;
    if (t == nullptr) return 0;
    return 0;
}