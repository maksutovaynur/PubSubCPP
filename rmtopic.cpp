#include <iostream>
#include <unistd.h>
#include <csignal>
#include "topic.hpp"
#include <cstdio>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];

const std::string TOPIC_NAME = "/clap0";

int main() {
    std::cout << Topic::UI_SZ << "=ui|hdr=" << Topic::HDR_SZ << std::endl;
    if (Topic::remove(TOPIC_NAME)) std::cout << "Removed topic " << TOPIC_NAME << std::endl;
    return 0;
}