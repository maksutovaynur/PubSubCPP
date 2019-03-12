//
// Created by aynur on 12.03.19.
//

#include <string>
#include <iostream>


struct A{
    int x;
    int y;
    std::string u;
};

struct B{
    A s;
    int r;
};

B createB(A &s){
    B a;
    a.s = s;
    a.r = -1;
    return a;
}

int main(int argv, char ** argc){
    B a;
    {
        A s;
        s.x = 0;
        s.y = 15;
        s.u = "Hello";
        a = createB(s);
    }
    std::cout << a.s.x << " " << a.s.y << " " << a.s.u << std::endl;
    return 0;
}