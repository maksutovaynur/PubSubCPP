#include <iostream>
#include "../lib/topic.hpp"
#include <cstdio>

const unsigned int MSG_SIZE = 256;
char msg[MSG_SIZE + 1];

int main() {
    auto t = Topic::spawn_create("/mytopic_name", MSG_SIZE, 10);
    std::cout << "Started topic: " << (bool)(t != nullptr) << std::endl << "Enter 0 to pub, 1 to sub" << std::endl;
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
