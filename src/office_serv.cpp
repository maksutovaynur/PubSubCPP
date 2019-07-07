#include "../lib/topic.hpp"


struct Question{
    int a;
    int b;
};

int main(){
    Office::Ptr off = Office::open_create("/my_funny_office1", sizeof(Question), sizeof(int));  // Open existing box
    if (nullptr == off){
        std::cout << "Cannot open or create office!" << std::endl;
        return 1;
    }
    Question q;
    int c;
    while (!tpc::interrupted){
        off->get_question(&q);
        c = q.a + q.b;
        std::cout << q.a << " + " << q.b << " = " << c << std::endl;
        off->put_answer(&c);
    }
    std::cout << "End of office work" << std::endl;
    return 0;
}