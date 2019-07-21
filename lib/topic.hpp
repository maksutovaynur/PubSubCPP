#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <cerrno>
#include <iostream>
#include "debug.hpp"

#ifdef DEBUG
#define DEBUG_MSG(str, lev) do {if (0 !=((DEBUG)&lev)) std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str, lev) 0;
//s#do { } while ( false )
#endif // DEBUG MSG

#ifndef PUBSUBCPP_TOPIC_H
#define PUBSUBCPP_TOPIC_H

using ui = unsigned long int;


int test(){
    return 42;
}


namespace tpc {
    volatile thread_local bool interrupted;

    bool Err(const std::string &str) {
        std::cout << str << std::endl;
        return false;
    }

    ui uiErr(const std::string &str) {
        std::cout << str << std::endl;
        return 0;
    }

    class SharedMemory {
    public:
        SharedMemory(const std::string &name, ui size) {
            this->name = name;
            this->fd = -1;
            this->size = size;
            this->data = MAP_FAILED;
        }

        bool exists() {
            if (fd >= 0) return true;
            fd = shm_open(name.c_str(), O_RDWR, 0777);
            if (fd < 0) return false;
            ::close(fd);
            return true;
        }

        bool create() {
            fd = shm_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0777);
            if (fd < 0) return false;
            DEBUG_MSG("Truncate " << size, DF2);
            ftruncate(fd, size);
            ::close(fd);
            return true;
        }

        bool open(bool ign_size) {
            if (MAP_FAILED != data) return true;
            if (!exists()) return tpc::Err("Shared memory doesn't exist");
            fd = shm_open(name.c_str(), O_RDWR, 0777);
            DEBUG_MSG("Shmem " << name << " open result " << fd, DF2);
            if (fd < 0) return false;
            {
                struct stat info;
                fstat(fd, &info);
                if (info.st_size != size) {
                    if (!ign_size) {
                        ::close(fd);
                        return tpc::Err("Shared memory size doesn't match");
                    } else size = info.st_size;
                    DEBUG_MSG("Now shmem size is " << size, DF2);
                }
            }
            data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            DEBUG_MSG("Shmem " << name << " mapped", DF2);
            return MAP_FAILED != data;
        }

        bool is_open() {
            return MAP_FAILED != data;
        }

        bool close() {
            if (!is_open()) return true;
            munmap(data, size);
            ::close(fd);
            data = MAP_FAILED;
            fd = -1;
            return true;
        }

        bool remove() {
            int res = shm_unlink(name.c_str());
            DEBUG_MSG("Shmem was unlinked (means deleted) with status " << res
            << ", errno=" << errno, DF2);
            return true;
        }

        void *data;
        std::string name;
        ui size;
        int fd = -1;

        friend class Topic;
    };

    using Shm = std::shared_ptr<SharedMemory>;

    Shm ShmMake(const std::string &name, ui size) {
        return std::make_unique<SharedMemory>(name, size);
    }

    class Semaphore {
    public:
        explicit Semaphore(const std::string &name) {
            this->name = name;
            this->sem = SEM_FAILED;
        }

        bool exists() {
            if (sem != SEM_FAILED) return true;
            sem = sem_open(name.c_str(), O_RDWR);
            if (SEM_FAILED == sem) return false;
            sem_close(sem);
            sem = SEM_FAILED;
            return true;
        }

        bool create(int val) {
            sem = sem_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0777, val);
            if (SEM_FAILED == sem) return tpc::Err("Can't create semaphore: error in sem_open");
            sem_close(sem);
            sem = SEM_FAILED;
            return true;
        }

        bool open_create(int val) {
            if (SEM_FAILED != sem) return Err("Sem already opened");
            sem = sem_open(name.c_str(), O_RDWR | O_CREAT, 0777, val);
            if (SEM_FAILED == sem) return Err("Can't create semaphore: error in sem_open");
            return true;
        }

        bool open() {
            if (SEM_FAILED != sem) return true;
            if (!exists()) return tpc::Err("Semaphore doesn't exist");
            sem = sem_open(name.c_str(), O_RDWR);
            return SEM_FAILED != sem;
        }

        bool is_open() {
            return SEM_FAILED != sem;
        }

        bool close() {
            if (!is_open()) return true;
            sem_close(sem);
            sem = SEM_FAILED;
            return true;
        }

        bool remove() {
            sem_unlink(name.c_str());
            return true;
        }

        sem_t *sem;
        std::string name;
    };

    using Sem = std::shared_ptr<Semaphore>;

    Sem SemMake(const std::string &name) {
        return std::make_shared<Semaphore>(name);
    }

    class Lock {
    public:
        explicit Lock(sem_t *sem) {
            this->sem = sem;
            locked = -1 != sem_wait(sem);
        }

        ~Lock() {
            if (locked) sem_post(sem);
            locked = false;
        }

        bool locked = false;
        sem_t *sem;
    };

    class ReadersLock {
    public:
        ReadersLock(sem_t *sem, ui *counter, sem_t *cond) {
            this->counter = counter;
            this->cond = cond;
            this->sem = sem;
            auto l = Lock(sem);
            DEBUG_MSG(" Rcounter(c0): " << *counter, DF3);
            if (1 == ++*counter) {
                DEBUG_MSG("1reader", DF3);
                if (-1 == sem_wait(cond)) {
                    --*counter;
                    DEBUG_MSG("cannot lock writer's lock while reading", DF3);
                    locked = false;
                } else locked = true;
            } else locked = true;

            DEBUG_MSG("Rcounter(c1): " << *counter, DF3);
        }

        ~ReadersLock() {
            if (!locked) return;
            auto l = Lock(sem);
            DEBUG_MSG("Rcounter(d0): " << *counter, DF3);
            if (0 == --*counter) {
                DEBUG_MSG("0reader", DF3);
                sem_post(cond);
            }
            DEBUG_MSG("Rcounter(d1): " << *counter, DF3);
        }

        sem_t *sem, *cond;
        ui *counter;
        bool locked = false;
    };

    class WriterLock {
    public:
        WriterLock(sem_t *sem, ui *counter, sem_t **lim, ui lim_count) {
            this->lim = lim;
            auto l = Lock(sem);
            pos = (*counter)++;
            DEBUG_MSG("Writer pos: " << pos, DF2);
            *counter %= lim_count;
            DEBUG_MSG("Next: " << *counter, DF3);
            if (-1 == sem_wait(lim[*counter])) {
                (*counter)--;
                *counter %= lim_count;
                pos = *counter;
                DEBUG_MSG("cannot lock writer's lock while reading", DF3);
                locked = false;
            } else locked = true;
        }

        ~WriterLock() {
            DEBUG_MSG("Writer sem_post:" << pos, DF3);
            if (locked) sem_post(lim[pos]);
        }

        bool locked = false;
        sem_t **lim;
        ui pos;
    };

    class DataForSemaphoreArray {
    public:
        explicit DataForSemaphoreArray(ui count) {
            this->count = count;
            data = (sem_t **) malloc(count * sizeof(sem_t *));
        }

        ~DataForSemaphoreArray() {
            free(data);
        }

        ui count;
        sem_t **data;
    };

    using SemArr = std::shared_ptr<DataForSemaphoreArray>;

    SemArr SemArrMalloc(ui count) {
        return std::make_unique<DataForSemaphoreArray>(count);
    }

    static void signal_handler(int i) {
        interrupted = true;
        DEBUG_MSG("signal: " + std::to_string(i), DF3);
        //raise(SIGINT);
    }

    void init_system() {
        interrupted = false;
        signal(SIGINT, signal_handler);
        signal(SIGQUIT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGKILL, signal_handler);
        signal(SIGSTOP, signal_handler);
        siginterrupt(SIGINT, 1);
        siginterrupt(SIGQUIT, 1);
        siginterrupt(SIGTERM, 1);
        siginterrupt(SIGKILL, 1);
        siginterrupt(SIGSTOP, 1);
    }

    class RWLock{
    public:
        RWLock(sem_t *w_sem, sem_t *r_sem, ui *counter){
            state = state_free;
            this->counter = counter;
            this->w_sem = w_sem;
            this->r_sem = r_sem;
        }
        bool reader_lock(){
            auto l = tpc::Lock(r_sem);
            if (!l.locked) return false;
            if (1 == ++*counter) if (-1 == sem_wait(w_sem)) { --*counter; return false; }
            state = in_read;
            return true;
        }
        bool writer_lock(){
            if (-1 == sem_wait(w_sem)) return false;
            state = in_write;
            return true;
        }
        ~RWLock(){
            if (state==in_read) {
                auto l = tpc::Lock(r_sem);
                if (0 == --*counter) sem_post(w_sem);
            }
            else if (state==in_write) sem_post(w_sem);
        }
        sem_t *w_sem, *r_sem;
        enum {state_free, in_read, in_write} state;
        ui *counter;
    };


}

class Box{
public:
    using Ptr=std::shared_ptr<Box>;
    static Ptr create(const std::string &name, ui size){
        Ptr loc(new Box(name, size));
        if (loc->exists()) return nullptr;
        loc->remove();
        if (!loc->create() || !loc->open()) return nullptr;
        return loc;
    }
    static Ptr just_open(const std::string &name, ui size){
        Ptr loc(new Box(name, size));
        if (!loc->exists()) return nullptr;
        if (!loc->open()) return nullptr;
        return loc;
    }
    static Ptr open_create(const std::string &name, ui size){
        Ptr loc(new Box(name, size));
        if (!loc->exists()) {
            loc->remove();
            if (!loc->create()) return nullptr;
        }
        if (!loc->open()) return nullptr;
        return loc;
    }
    static bool remove(const std::string &name){
        auto b = Box(name, 0);
        return b.remove();
    }
    bool get(void* data, ui size){
        if (tpc::interrupted) return false;
        if (size > this->mysize) return false;
        sem_post(w_sem->sem);
        sem_wait(r_sem->sem);
        memcpy(data, mem->data, size);
        return true;
    }
    bool get(void* data){
        return get(data, mysize);
    }
    bool put(void* data, ui size){
        if (tpc::interrupted) return false;
        if (size > this->mysize) return false;
        sem_wait(w_sem->sem);
        memcpy(mem->data, data, size);
        sem_post(r_sem->sem);
        return true;
    }
    bool put(void* data){
        return put(data, mysize);
    }
    bool remove(){
        return r_sem->remove() && w_sem->remove() && mem->remove();
    }
    const std::string & get_name(){
        return name;
    }
private:
    Box(const std::string& name, ui size){
        this->name = name;
        this->mysize = size;
        r_sem = tpc::SemMake(name + "-R");
        w_sem = tpc::SemMake(name + "-W");
        mem = tpc::ShmMake(name, size);
    }
    bool exists(){
        return r_sem->exists() && w_sem->exists() && mem -> exists();
    }
    bool create(){
        return r_sem->create(0) && w_sem->create(0) && mem->create();
    }
    bool open(){
        return r_sem->open() && w_sem->open() && mem->open(false);
    }
    std::string name;
    ui mysize;
    tpc::Shm mem;
    tpc::Sem r_sem, w_sem;
};


class Variable{
public:
    using Ptr=std::shared_ptr<Variable>;
    static Ptr create(const std::string &name, ui size){
        Ptr var(new Variable(name, size));
        if (var->exists()) return nullptr;
        var->remove();
        if (!var->create() || !var->open()) return nullptr;
        return var;
    }
    static Ptr just_open(const std::string &name, ui size){
        Ptr var(new Variable(name, size));
        if (!var->exists()) return nullptr;
        if (!var->open()) return nullptr;
        return var;
    }
    static Ptr open_create(const std::string &name, ui size){
        Ptr var(new Variable(name, size));
        if (!var->exists()) {
            var->remove();
            if (!var->create()) return nullptr;
        }
        if (!var->open()) return nullptr;
        return var;
    }
    static bool remove(const std::string &name){
        auto b = Variable(name, 0);
        return b.remove();
    }
    bool read(const void *data, ui size){
        if (tpc::interrupted) return false;
        if (size > this->mysize) return false;
        auto l = tpc::RWLock(w_sem->sem, r_sem->sem, counter);
        if (!l.reader_lock()) return false;
        memcpy((void *)data, mem->data, size);
        return true;
    }
    bool read(const void *data){
        return read(data, mysize);
    }
    bool write(const void *data, ui size){
        if (tpc::interrupted) return false;
        if (size > this->mysize) return false;
        auto l = tpc::RWLock(w_sem->sem, r_sem->sem, counter);
        if (!l.writer_lock()) return false;
        memcpy((void *)mem->data, data, size);
        return true;
    }
    bool write(const void *data){
        return write(data, mysize);
    }
    bool remove(){
        return r_sem->remove() && w_sem->remove() && mem->remove();
    }
    const std::string & get_name(){
        return name;
    }
private:
    Variable(const std::string& name, ui size){
        this->name = name;
        this->mysize = size;
        r_sem = tpc::SemMake(name + "-varR");
        w_sem = tpc::SemMake(name + "-varW");
        mem = tpc::ShmMake(name, size + sizeof(ui));
    }
    bool exists(){
        return r_sem->exists() && w_sem->exists() && mem -> exists();
    }
    bool create(){
        return r_sem->create(1) && w_sem->create(1) && mem->create();
    }
    bool open(){
        bool res = r_sem->open() && w_sem->open() && mem->open(false);
        if (res) counter = (ui *) (mysize + (char *)mem->data);
        return res;
    }
    ui mysize;
    ui *counter;
    std::string name;
    tpc::Shm mem;
    tpc::Sem r_sem, w_sem;
};


class Office{
public:
    using Ptr = std::shared_ptr<Office>;
    static Ptr open_create(const std::string &name, ui in_size, ui out_size) {
        auto in = Box::open_create(in_name(name), in_size);
        auto out = Box::open_create(out_name(name), out_size);
        if (in == nullptr || out == nullptr) return nullptr;
        std::shared_ptr<Office> off(new Office(name, in, out));
        return off;
    }

    static Ptr create(const std::string &name, ui in_size, ui out_size) {
        auto in = Box::create(in_name(name), in_size);
        auto out = Box::create(out_name(name), out_size);
        if (in == nullptr || out == nullptr) return nullptr;
        std::shared_ptr<Office> off(new Office(name, in, out));
        return off;
    }

    static Ptr just_open(const std::string &name, ui in_size, ui out_size) {
        auto in = Box::just_open(in_name(name), in_size);
        auto out = Box::just_open(out_name(name), out_size);
        if (in == nullptr || out == nullptr) return nullptr;
        std::shared_ptr<Office> off(new Office(name, in, out));
        return off;
    }

    static bool remove(const std::string &name){
        return Box::remove(in_name(name)) && Box::remove(out_name(name));
    }

    bool ask(void *question, ui size){
        return input->put(question, size);
    }
    bool wait(void *answer, ui size){
        return output->get(answer, size);
    }
    bool look(void *question, ui size){
        return input->get(question, size);
    }
    bool answer(void *answer, ui size){
        return output->put(answer, size);
    }

    bool ask(void *question){
        return input->put(question);
    }
    bool get_answer(void *answer){
        return output->get(answer);
    }
    bool get_question(void *question){
        return input->get(question);
    }
    bool put_answer(void *answer){
        return output->put(answer);
    }
    const std::string & get_name(){
        return myname;
    }
private:
    static std::string in_name(const std::string &name){
        return name + "-I";
    }
    static std::string out_name(const std::string &name){
        return name + "-O";
    }
    Office(const std::string &name, Box::Ptr& in, Box::Ptr& out) {
        input = in;
        output = out;
        myname = name;
    }

    Box::Ptr input;
    Box::Ptr output;
    std::string myname;
};


class Topic {
public:
    static bool was_interrupted() {
        return tpc::interrupted;
    }

    using Ptr = std::shared_ptr<Topic>;

    static bool remove(const std::string &name) {
        std::shared_ptr<Topic> t(new Topic(name, 0, 0));
        return t->remove();
    }

    static Ptr spawn(const std::string &name, ui msg_size, ui msg_count) {
        std::shared_ptr<Topic> t(new Topic(name, msg_size, msg_count));
        if (t->start(false, false, false)) return t;
        else return nullptr;
    }

    static Ptr spawn(const std::string &name, ui msg_size) {
        std::shared_ptr<Topic> t(new Topic(name, msg_size, 0));
        if (t->start(false, false, true)) return t;
        else return nullptr;
    }

    static Ptr spawn_create(const std::string &name, ui msg_size, ui msg_count) {
        std::shared_ptr<Topic> t(new Topic(name, msg_size, msg_count));
        if (t->start(true, false, false)) return t;
        else return nullptr;
    }

    static Ptr spawn(const std::string &name) {
        std::shared_ptr<Topic> t(new Topic(name, 0, 0));
        if (t == nullptr) return nullptr;
        if (t->start(false, true, true)) return t;
        else return nullptr;
    }

    ui pub(const void *msg, ui size) {
        if (tpc::interrupted)
            return tpc::uiErr("Pub " + name + " was interrupted");
        DEBUG_MSG("Entered pub in " + name, DF4);
        if (size > msg_size)
            return tpc::uiErr("Pub error: MsgSize is bigger than fixed for topic");
        auto l = tpc::WriterLock(nlock, WposSRC, wlocks->data, msg_count);
        if (!l.locked)
            return tpc::uiErr("Pub error: WriterLock didn't lock");
        Wpos = l.pos;
        memcpy(data[Wpos], msg, size);
        *Msizes[Wpos] = size;
        return size;
    }

    ui pub(const void *msg) {
        return pub(msg, msg_size);
    }

    ui sub(const void *msg) {
        DEBUG_MSG("Entered sub in " + name, DF4);
        if (tpc::interrupted) return 0;
        DEBUG_MSG("Reader pos: " + std::to_string(Rpos), DF4);
        auto l = tpc::ReadersLock(rlocks->data[Rpos], Rcounters[Rpos], wlocks->data[Rpos]);
        if (!l.locked) return 0;
        ui sz = *Msizes[Rpos];
        memcpy((void *) msg, data[Rpos], sz);
        Rpos = (Rpos + 1) % msg_count;
        return sz;
    }

    ui get_msg_size() {
        return msg_size;
    }

    ui get_msg_count() {
        return msg_count;
    }

    ui get_shmem_size() {
        return full_size;
    }

    bool is_ready() {
        return steady;
    }

    const std::string &get_name() {
        return name;
    }

    struct Header {
        ui msg_size;
        ui msg_count;
        ui writer_pos;
    };

    static const ui DATA_START = 32;
    static const ui UI_SZ = sizeof(ui);
    static const ui HDR_SZ = sizeof(Header);

private:
    Topic(const std::string &name, ui msg_size, ui msg_count) {
        tpc::init_system();
        this->name = name;
        this->msg_size = msg_size;
        this->msg_count = msg_count;
        full_size = DATA_START + (msg_size + UI_SZ * 2) * msg_count;
        DEBUG_MSG("Full size " << full_size, DF5);
        memory = tpc::ShmMake(name, full_size);
        semCreate = tpc::SemMake(name + "--C");
        steady = false;
    }

    ui getWpos() {
        auto l = tpc::Lock(nlock);
        ui pos = *WposSRC;
        return pos;
    }

    bool remove() {
        if (semN != nullptr) semN->remove();
        for (auto &&i : semW) i->remove();
        for (auto &&i : semR) i->remove();
        if (memory != nullptr) {
            memory->remove();
            DEBUG_MSG("Memory was removed", DF5);
        }
        return true;
    }

    bool start(bool create, bool ign_size, bool ign_count) {
        if (msg_count <= 1) return false;
        DEBUG_MSG("Will start topic " << name << " with flags: create[" << create
        << "], ign_size[" << ign_size << "], ign_count[" << ign_count << "]", DF5);
        if (steady) return true;
        char *mp, *mpd;
        semN = tpc::SemMake(name + "--n");
        bool existed = true;
        if (!semCreate->open_create(1)) return tpc::Err("Unable to create Shmem lock");
        {
            auto l = tpc::Lock(semCreate->sem);
            if (!memory->exists()) {
                DEBUG_MSG("Topic didnt exist " << name, DF5);
                existed = false;
                if (!create)
                    return tpc::Err("Topic doesn't exist; do nothing.");
                if (!memory->create())
                    return tpc::Err("Can't create topic.");
                if (!memory->open(false))
                    return tpc::Err("Topic created, but errors occured while opening");
                mp = (char *) memory->data;
                mpd = mp + DATA_START;
                auto hdr = (Header *) mp;
                hdr->msg_count = msg_count;
                hdr->msg_size = msg_size;
                hdr->writer_pos = 0;
                WposSRC = &(hdr->writer_pos);
                Rpos = 0;
                semR.clear();
                semW.clear();
                for (ui i = 0; i < msg_count; i++)
                    semR.push_back(tpc::SemMake(name + "--r" + std::to_string(i)));
                for (ui i = 0; i < msg_count; i++)
                    semW.push_back(tpc::SemMake(name + "--w" + std::to_string(i)));
                create_sems();
                Rcounters.clear();
                for (int i = 0; i < msg_count; i++)
                    Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2)));
                for (int i = 0; i < msg_count; i++)
                    Msizes.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2) + UI_SZ));
                DEBUG_MSG("Just before Rcounters=0", DF5);
                for (int i = 0; i < msg_count; i++)
                    (*Rcounters[i]) = 0;
                DEBUG_MSG("Just after Rcounters=0", DF5);
            }
        }
        if (existed) {
            if (!memory->open(true))
                return tpc::Err("Topic existed, but errors occured while opening");
            DEBUG_MSG("Topic existed " << name, DF5);
            mp = (char *) memory->data;
            mpd = mp + DATA_START;
            auto hdr = (Header *) mp;
            DEBUG_MSG("Before work with shmem hdr", DF5);
            if (msg_size != hdr->msg_size) {
                if (!ign_size)
                    return tpc::Err(
                        "Proposed message size != existing topic message size: found "
                        + std::to_string(msg_size) + " but expected "
                        + std::to_string(hdr->msg_size));
                msg_size = hdr->msg_size;
            }
            if (msg_count != hdr->msg_count) {
                if (!ign_count)
                    return tpc::Err("Given message != existing topic message count");
                msg_count = hdr->msg_count;
            }
            WposSRC = &(hdr->writer_pos);
            DEBUG_MSG("Just after work with shmem hdr", DF5);
            Rpos = 0;
            Rcounters.clear();
            Msizes.clear();
            for (int i = 0; i < msg_count; i++)
                Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2)));
            for (int i = 0; i < msg_count; i++)
                Msizes.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2) + UI_SZ));
            DEBUG_MSG("Rconters/Msizes", DF5);
            semR.clear();
            semW.clear();
            for (ui i = 0; i < msg_count; i++)
                semR.push_back(tpc::SemMake(name + "--r" + std::to_string(i)));
            for (ui i = 0; i < msg_count; i++)
                semW.push_back(tpc::SemMake(name + "--w" + std::to_string(i)));
        }
        if (msg_size <= 0) return tpc::Err("Message size should be > 0");
        if (msg_count <= 0) return tpc::Err("Message count should be > 0");
        full_size = memory->size;
        DEBUG_MSG("Just before sem open: msg_count=" << msg_count
        << ", msg_size=" << msg_size, DF5);
        open_sems();
        DEBUG_MSG("Opened sems", DF5);
        data.clear();
        for (int i = 0; i < msg_count; i++)
            data.push_back(mpd + i * (msg_size + UI_SZ * 2) + UI_SZ * 2);
        Rpos = getWpos();
        DEBUG_MSG("Topic " << name << " successfully opened", DF5);
        return true;
    }

    bool create_sems() {
        semN->remove();
        if (!semN->create(1)) return tpc::Err("Can't create W_POS semaphore");
        for (auto &&i : semR) i->remove();
        for (auto &&i : semW) i->remove();
        if ((!semR[0]->create(1)) || (!semW[0]->create(0)))
            return tpc::Err("Can't create W/R semaphore(0)");
        for (int i = 1; i < msg_count; i++)
            if (!semR[i]->create(1) || !semW[i]->create(1))
                return tpc::Err("Can't create W/R semaphore(all)");
        return true;
    }

    bool open_sems() {
        DEBUG_MSG("Enter open_sems()", DF5);
        if (!semN->open()) return tpc::Err("Can't open W_POS semaphore");
        for (int i = 0; i < msg_count; i++)
            if ((!semR[i]->open()) || (!semW[i]->open()))
                return tpc::Err("Can't open W/R semaphore");
        rlocks = tpc::SemArrMalloc(msg_count);
        wlocks = tpc::SemArrMalloc(msg_count);
        for (int i = 0; i < msg_count; i++) {
            rlocks->data[i] = semR[i]->sem;
            wlocks->data[i] = semW[i]->sem;
        }
        nlock = semN->sem;
        return true;
    }

    bool steady;
    tpc::Shm memory;
    tpc::Sem semN, semCreate;
    std::vector<tpc::Sem> semW, semR;
    tpc::SemArr wlocks, rlocks;
    sem_t *nlock;
    std::vector<char *> data;
    std::vector<ui *> Rcounters, Msizes;
    ui Wpos, *WposSRC, Rpos;
    std::string name;
    ui msg_size, msg_count, full_size;
};

#endif