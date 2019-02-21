#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "topic.hpp"
#include <stdio.h>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];

int main() {
    auto t = topic::Topic::spawn_create("/clap0", MSG_SIZE, 10);
    std::cout << "Started topic: " << (bool)(t != nullptr) << std::endl << "Enter 0 to pub, 1 to sub" << std::endl;
    if (t == nullptr) return 0;
    int i;
    std::cin >> i;
    if (i == 0){
        std::string m_base;
        int counter = 0;
        std::cout << "Enter string to pub" << std::endl;
        std::cin >> m_base;
        while (!topic::interrupted) {
            sprintf(msg, "%s%d", m_base.c_str(), counter++);
            t->pub(msg);
            std::cout << msg << "|end" << std::endl;
            sleep(1);
        }
    }else{
        while (!topic::interrupted){
            t->sub(msg);
            std::cout << msg << "|end" << std::endl;
        }
    }
    return 0;
}