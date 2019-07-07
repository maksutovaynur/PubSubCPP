#include "../lib/topic.hpp"
#include <ctime>
#include <iostream>

int main(int argv, char ** argc){
    std::time_t result = std::time(nullptr);
    std::cout << std::asctime(std::localtime(&result))
              << result << " Start\n";

    for (int i = 0; i < 1000000; i ++)
        service::util::resp_topic_name("me_funny_serv_name", 223438, 2334203);

    std::time_t eresult = std::time(nullptr);
    std::cout << std::asctime(std::localtime(&eresult))
              << result << "End\n";
    return 0;
}