#include <iostream>
#include <unistd.h>
#include <csignal>
#include "topic.hpp"
#include <cstdio>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];
volatile bool interrupted;

void signal_handler(int i){
    interrupted = true;
}

std::string TOPIC_NAME = "/clap0";

int main() {
    signal(SIGINT, signal_handler);
    if (topic::Topic::remove(TOPIC_NAME)) std::cout << "Removed topic " << TOPIC_NAME << std::endl;
    return 0;
}