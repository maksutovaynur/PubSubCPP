#include <string>
#include <map>
#include <iostream>
#include "topic.hpp"

extern "C" {
    using TopPtr = Topic::Ptr;

    size_t SZ = sizeof(TopPtr);

    size_t TopSZ(){
        return SZ;
    }

    int str_len(const char * c){
        return strlen(c);
    }

    struct T {
    public:
        explicit T(TopPtr &&t) {
            ptr = t;
        }
        TopPtr ptr;
    };

    bool is_null(T* t){
        return t->ptr == nullptr;
    }

    ui pub(T* t, const char *msg, ui size) {
        if (size == 0) return t->ptr->pub(msg);
        else return t->ptr->pub(msg, size);
    }

    ui sub(T* t, const char *msg) {
        return t->ptr->sub(msg);
    }

    ui shmem_size(T* t){
        return t->ptr->get_shmem_size();
    }

    ui msg_size(T* t){
        auto s = t->ptr->get_msg_size();
        std::cout << "T " << t << ", size " << s << std::endl;
        return s;
    }

    ui msg_count(T* t){
        return t->ptr->get_msg_count();
    }

    bool spawn_topic(T* t, const char * name, ui size, ui count){
        std::string n(name);
        TopPtr ptr;
        if (size == 0) ptr = Topic::spawn(n);
        else if (count == 0) ptr = Topic::spawn(n, size);
        else ptr = Topic::spawn(n, size, count);
        t->ptr = ptr;
        return ptr != nullptr;
    }

    bool create_topic(T* t, const char * name, ui msg_size, ui msg_count){
        std::string n(name);
        auto ptr = Topic::spawn_create(n, msg_size, msg_count);
        std::cout << "T " << t << ", name " << name << ", size " << msg_size
        << ", count " << msg_count << std::endl;
        t->ptr = ptr;
        return ptr != nullptr;
    }
    bool remove_topic(const char * name){
        std::string n(name);
        return Topic::remove(n);
    }
//
//    int bb(int a){ return a*a;};
//
//    void x(){
//        TopPtr t = Topic::spawn("aa");
//        t->get_msg_size();
//    }
};


void __attribute__ ((constructor)) initLibrary(void) {
    //
    // Function that is called when the library is loaded
    //
    printf("Library is initialized\n");
}

void __attribute__ ((destructor)) cleanUpLibrary(void) {
    //
    // Function that is called when the library is »closed«.
    //
    printf("Library is exited\n");
}