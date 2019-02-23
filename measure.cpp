#include <iostream>
#include "topic.hpp"
#include <stdio.h>

int main(int argc, char** args) {
    ui msg_size = 256;
    ui msg_count = 10;
    std::string name = "/clap0";
    int sender = 0;
    int create = 0;
    Topic::TopPtr t;
    if (argc > 1) name = std::string(args[1]);
    if (argc > 2) sscanf(args[2], "%d", &sender);
    if (argc > 3) sscanf(args[3], "%d", &create);
    if (argc > 4) sscanf(args[4], "%u", &msg_size);
    char msg[msg_size];
    if (argc > 5) sscanf(args[5], "%u", &msg_count);
    if (argc > 6) sscanf(args[6], "%s", msg);
    if (create) t = Topic::spawn_create("/clap0", MSG_SIZE, 10);
    if (t == nullptr) return 0;
    int i;
    std::cin >> i;
    if (i == 0){
        std::string m_base;
        int counter = 0;
        std::cout << "Enter string to pub" << std::endl;
        std::cin >> m_base;
        while (!Topic::was_interrupted()) {
            sprintf(msg, "%s%d", m_base.c_str(), counter++);
            t->pub(msg);
            std::cout << msg << "|end" << std::endl;
            sleep(1);
        }
    }else{
        while (!Topic::was_interrupted()){
            t->sub(msg);
            std::cout << msg << "|end" << std::endl;
        }
    }
    return 0;
}