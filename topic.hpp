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
            DEBUG_MSG("Shmem was unlinked (means deleted) with status " << res << ", errno=" << errno, DF2);
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
        b.remove();
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
        return r_sem->remove() && mem->remove();
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
        if (tpc::interrupted) return tpc::uiErr("Pub " + name + " was interrupted");
        DEBUG_MSG("Entered pub in " + name, DF4);
        if (size > msg_size) return tpc::uiErr("Pub error: MsgSize is bigger than fixed for topic");
        auto l = tpc::WriterLock(nlock, WposSRC, wlocks->data, msg_count);
        if (!l.locked) return tpc::uiErr("Pub error: WriterLock didn't lock");
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
        DEBUG_MSG("Will start topic " << name << " with flags: create[" << create << "], ign_size[" << ign_size
                                      << "], ign_count[" << ign_count << "]", DF5);
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
                if (!create) return tpc::Err("Topic doesn't exist; do nothing.");
                if (!memory->create()) return tpc::Err("Can't create topic.");
                if (!memory->open(false)) return tpc::Err("Topic created, but errors occured while opening");
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
                for (ui i = 0; i < msg_count; i++) semR.push_back(tpc::SemMake(name + "--r" + std::to_string(i)));
                for (ui i = 0; i < msg_count; i++) semW.push_back(tpc::SemMake(name + "--w" + std::to_string(i)));
                create_sems();
                Rcounters.clear();
                for (int i = 0; i < msg_count; i++) Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2)));
                for (int i = 0; i < msg_count; i++) Msizes.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2) + UI_SZ));
                DEBUG_MSG("Just before Rcounters=0", DF5);
                for (int i = 0; i < msg_count; i++) (*Rcounters[i]) = 0;
                DEBUG_MSG("Just after Rcounters=0", DF5);
            }
        }
        if (existed) {
            if (!memory->open(true)) return tpc::Err("Topic existed, but errors occured while opening");
            DEBUG_MSG("Topic existed " << name, DF5);
            mp = (char *) memory->data;
            mpd = mp + DATA_START;
            auto hdr = (Header *) mp;
            DEBUG_MSG("Before work with shmem hdr", DF5);
            if (msg_size != hdr->msg_size) {
                if (!ign_size) return tpc::Err(
                            "Proposed message size doesn't match existing topic message size: found " +
                            std::to_string(msg_size) + " but expected " + std::to_string(hdr->msg_size));
                msg_size = hdr->msg_size;
            }
            if (msg_count != hdr->msg_count) {
                if (!ign_count) return tpc::Err("Proposed message count doesn't match existing topic message count");
                msg_count = hdr->msg_count;
            }
            WposSRC = &(hdr->writer_pos);
            DEBUG_MSG("Just after work with shmem hdr", DF5);
            Rpos = 0;
            Rcounters.clear();
            Msizes.clear();
            for (int i = 0; i < msg_count; i++) Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2)));
            for (int i = 0; i < msg_count; i++) Msizes.push_back((ui *) (mpd + i * (msg_size + UI_SZ * 2) + UI_SZ));
            DEBUG_MSG("Rconters/Msizes", DF5);
            semR.clear();
            semW.clear();
            for (ui i = 0; i < msg_count; i++) semR.push_back(tpc::SemMake(name + "--r" + std::to_string(i)));
            for (ui i = 0; i < msg_count; i++) semW.push_back(tpc::SemMake(name + "--w" + std::to_string(i)));
        }
        if (msg_size <= 0) return tpc::Err("Message size should be > 0");
        if (msg_count <= 0) return tpc::Err("Message count should be > 0");
        full_size = memory->size;
        DEBUG_MSG("Just before sem open: msg_count=" << msg_count << ", msg_size=" << msg_size, DF5);
        open_sems();
        DEBUG_MSG("Opened sems", DF5);
        data.clear();
        for (int i = 0; i < msg_count; i++) data.push_back(mpd + i * (msg_size + UI_SZ * 2) + UI_SZ * 2);
        Rpos = getWpos();
        DEBUG_MSG("Topic " << name << " successfully opened", DF5);
        return true;
    }

    bool create_sems() {
        semN->remove();
        if (!semN->create(1)) return tpc::Err("Can't create W_POS semaphore");
        for (auto &&i : semR) i->remove();
        for (auto &&i : semW) i->remove();
        if ((!semR[0]->create(1)) || (!semW[0]->create(0))) return tpc::Err("Can't create W/R semaphore(0)");
        for (int i = 1; i < msg_count; i++)
            if (!semR[i]->create(1) || !semW[i]->create(1)) return tpc::Err("Can't create W/R semaphore(all)");
        return true;
    }

    bool open_sems() {
        DEBUG_MSG("Enter open_sems()", DF5);
        if (!semN->open()) return tpc::Err("Can't open W_POS semaphore");
        for (int i = 0; i < msg_count; i++)
            if ((!semR[i]->open()) || (!semW[i]->open())) return tpc::Err("Can't open W/R semaphore");
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


namespace service {
    namespace util {

        struct ReqHdr {
            int pid;
            ui qid;
            bool ackn;
        };

        template<std::size_t size_in>
        struct ReqMsg {
            ReqHdr hdr;
            char body[size_in];
        };

        struct RespHdr {
            int pid;
            ui qid;
            bool success;
        };

        template<std::size_t size_out>
        struct RespMsg {
            RespHdr hdr;
            char body[size_out];
        };

        static std::string name_in(const std::string &name) {
            return name + "--sin";
        }

        static std::string name_out(const std::string &name) {
            return name + "--sout";
        }

        static bool remove(const std::string &name) {
            bool b1 = Topic::remove(name_in(name));
            bool b2 = Topic::remove(name_out(name));
            return b1 && b2;
        }

        template<std::size_t size_in, std::size_t size_out>
        class Request {
        public:
            using Ptr = std::shared_ptr<Request>;

            static Ptr create(Topic::Ptr in, Topic::Ptr out) {
                return std::make_shared<Request>(in, out);
            }

            bool requires_answer() {
                return req.hdr.ackn && (!answered);
            }

            ui size() {
                return dsz;
            }

            void *data() {
                return &req.body;
            }

            ui answer(const void *data, ui size) {
                if (size > size_out) return 0;
                memcpy(&resp.body, data, size);
                resp.hdr.success = true;
                ui result = out->pub(&resp, sizeof(util::RespHdr) + size);
                if (result > 0) answered = true;
                return result;
            }

            ui answer(const void *data) {
                return answer(data, size_out);
            }

            bool deny(){
                resp.hdr.success = false;
                ui result = out->pub(&resp, sizeof(util::RespHdr));
                if (result > 0) answered = true;
                return answered;
            }

            Request(const Topic::Ptr &in, const Topic::Ptr &out) {
                DEBUG_MSG("Enter Request constructor", DF2);
                dsz = in->sub(&req) - sizeof(util::ReqHdr);
                resp.hdr.qid = req.hdr.qid;
                resp.hdr.pid = req.hdr.pid;
                resp.hdr.success = true;
                answered = false;
                this->out = out;
                DEBUG_MSG("Finish Request constructor", DF2);
            }

            ~Request() {
                if (requires_answer()) {
                    resp.hdr.success = false;
                    out->pub(&resp, sizeof(util::RespHdr));
                }
            }

        private:
            Topic::Ptr out;
            util::ReqMsg<size_in> req;
            util::RespMsg<size_out> resp;
            bool answered;
            ui dsz;
        };

        template<std::size_t size_in, std::size_t size_out>
        class AsyncServer {
        public:
            using Ptr = std::shared_ptr<AsyncServer>;

            static Ptr create(const std::string &name, ui in_msg_cnt, ui out_msg_cnt) {
                remove(name);
                auto in = Topic::spawn_create(name_in(name), sizeof(util::ReqMsg<size_in>), in_msg_cnt);
                auto out = Topic::spawn_create(name_out(name), sizeof(util::RespMsg<size_out>), out_msg_cnt);
                if (nullptr == in || nullptr == out) return nullptr;
                return std::make_shared<AsyncServer>(in, out);
            }

            typename Request<size_in, size_out>::Ptr wait_request() {
                if (tpc::interrupted) return nullptr;
                return Request<size_in, size_out>::create(in, out);
            }

            AsyncServer(const Topic::Ptr &in, const Topic::Ptr &out) {
                this->in = in;
                this->out = out;
            }

        private:
            Topic::Ptr in, out;
        };

        template<std::size_t size_in, std::size_t size_out>
        class SyncClient {
        public:
            using Ptr = std::shared_ptr<SyncClient>;

            static Ptr create(std::string &name) {
                auto in = Topic::spawn(name_in(name), sizeof(ReqHdr) + size_in);
                auto out = Topic::spawn(name_out(name), sizeof(RespHdr) + size_out);
                if (nullptr == in || nullptr == out) return nullptr;
                return std::make_shared<SyncClient>(in, out);
            }

            bool inform(void *request) {
                return _ask(request, size_in, false);
            }

            bool inform(void *request, ui request_size) {
                return _ask(request, request_size, false);
            }

            ui ask(void *request, void *response) {
                return ask(request, size_in, response);
            }

            ui ask(void *request, ui request_size, void *response) {
                if (!_ask(request, request_size, true)) return 0;
                ui result;
                do {
                    result = out->sub(&resp);
                    if (result == 0) continue;
                } while (resp.hdr.pid != req.hdr.pid || resp.hdr.qid != req.hdr.qid);
                if (!resp.hdr.success) return 0;
                result -= sizeof(RespHdr);
                memcpy(response, &resp.body, result);
                return result;
            }

            SyncClient(Topic::Ptr &in, Topic::Ptr &out) {
                this->in = in;
                this->out = out;
                req.hdr.pid = getpid();
                req.hdr.qid = 0;
            }

        private:
            bool _ask(void *request, ui request_size, bool ackn) {
                req.hdr.ackn = ackn;
                DEBUG_MSG("_ask: ackn = " + std::to_string(ackn), DF6);
                if (request_size > size_in) return false;
                memcpy(&req.body, request, request_size);
                ui result = in->pub(&req, sizeof(ReqHdr) + request_size);
                DEBUG_MSG("_ask: pub result = " + std::to_string(result), DF5);
                return result != 0;
            }

            Topic::Ptr in, out;
            ReqMsg<size_in> req;
            RespMsg<size_out> resp;
        };
        std::string resp_topic_name(const std::string &serv_name, int pid, int rnd){
            return serv_name + "--P" + std::to_string(pid) + "-R" + std::to_string(rnd);
        }

//        template<std::size_t size_in, std::size_t size_out>
//        class Client{
//            using Cli=std::shared_ptr<Client>;
//            Cli create(std::string & name){
//                auto cli = Client(name);
//            }
//            Client(const std::string & name){
//                pid = getpid();
//                rnd = random();
//                this->name = resp_topic_name(name, pid, rnd);
//                sem = tpc::SemMake(name + "-C");
//                sem->open_create(1);
//                loc = tpc::LocMake(this->name, size_out);
//                topic = Topic::spawn(name, sizeof(ReqHdr) + size_in);
//            }
//            bool open(){
//                if (!sem->is_open() )return false;
//                auto l = tpc::Lock(sem->sem);
//
//            }
//            std::string name;
//            tpc::Sem sem;
//            tpc::Loc loc;
//            Topic::Ptr topic;
//            int pid;
//            int rnd;
//
//        };

    }

    template<std::size_t size_in, std::size_t size_out>
    static typename util::AsyncServer<size_in, size_out>::Ptr
    create_async_server(std::string &name, ui in_msg_cnt, ui out_msg_cnt) {
        return util::AsyncServer<size_in, size_out>::create(name, in_msg_cnt, out_msg_cnt);
    };

    template<std::size_t size_in, std::size_t size_out>
    static typename util::SyncClient<size_in, size_out>::Ptr create_sync_client(std::string &name) {
        return util::SyncClient<size_in, size_out>::create(name);
    };

    static bool remove(std::string &name) {
        return util::remove(name);
    };


}
#endif //PUBSUBCPP2_TOPIC_H

