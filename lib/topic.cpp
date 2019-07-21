#include <string>
#include <map>
#include <iostream>
#include "topic.hpp"

extern "C" {
    int str_len(const char * c){return strlen(c);}

    using VarPtr = Variable::Ptr;
    size_t VarPtrSize = sizeof(VarPtr);
    size_t varPtrSize(){ return VarPtrSize; }
    struct VarStruct { public: explicit VarStruct(VarPtr &&v) {ptr = v;} VarPtr ptr;};
    bool varIsNull(VarStruct *v){return v->ptr == nullptr;}
    bool varRead(VarStruct *v, const char *msg, ui size){
        if (size == 0) return v->ptr->read(msg);
        return v->ptr->read(msg, size);
    }
    bool varWrite(VarStruct *v, const char *msg, ui size){
        if (size == 0) return v->ptr->write(msg);
        return v->ptr->write(msg, size);
    }
    bool varCreate(VarStruct *v, const char* name, ui size){
        std::string n(name);
        VarPtr ptr = Variable::open_create(n, size);
        v->ptr = ptr;
        return ptr != nullptr;
    }
    bool varOpen(VarStruct *v, const char* name, ui size){
        std::string n(name);
        VarPtr ptr = Variable::just_open(n, size);
        v->ptr = ptr;
        return ptr != nullptr;
    }
    bool varRemove(const char *name){
        std::string n(name);
        return Variable::remove(n);
    }

    using TopPtr = Topic::Ptr;
    size_t TopPtrSize = sizeof(TopPtr);
    size_t topPtrSize(){ return TopPtrSize;}
    struct TopStruct { public: explicit TopStruct(TopPtr &&t) {ptr = t;} TopPtr ptr;};
    bool topicIsNull(TopStruct *t){return t->ptr == nullptr;}
    ui topicShmemSize(TopStruct *t){return t->ptr->get_shmem_size();}
    ui topicMsgSize(TopStruct *t){return t->ptr->get_msg_size();}
    ui topicMsgCount(TopStruct *t){return t->ptr->get_msg_count();}
    ui topicPub(TopStruct *t, const char *msg, ui size) {
        if (size == 0) return t->ptr->pub(msg);
        else return t->ptr->pub(msg, size);
    }
    ui topicSub(TopStruct *t, const char *msg) {return t->ptr->sub(msg);}
    bool topicSpawn(TopStruct *t, const char *name, ui size, ui count){
        std::string n(name);
        TopPtr ptr;
        if (size == 0) ptr = Topic::spawn(n);
        else if (count == 0) ptr = Topic::spawn(n, size);
        else ptr = Topic::spawn(n, size, count);
        t->ptr = ptr;
        return ptr != nullptr;
    }
    bool topicCreate(TopStruct *t, const char *name, ui msg_size, ui msg_count){
        std::string n(name);
        auto ptr = Topic::spawn_create(n, msg_size, msg_count);
        std::cout << "T " << t << ", name " << name << ", size " << msg_size
        << ", count " << msg_count << std::endl;
        t->ptr = ptr;
        return ptr != nullptr;
    }
    bool topicRemove(const char *name){
        std::string n(name);
        return Topic::remove(n);
    }
};


void __attribute__ ((constructor)) initLibrary(void) {
    printf("PUBSUB init\n");
}

void __attribute__ ((destructor)) cleanUpLibrary(void) {
    printf("PUBSUB exit\n");
}