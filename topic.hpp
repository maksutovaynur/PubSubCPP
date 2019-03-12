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
#include "debug.hpp"

#ifdef DEBUG
#define DEBUG_MSG(str, lev) do { if (lev & DEBUG > 0) std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str, lev) do { } while ( false )
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
            if (fd < 0) return false;
            {
                struct stat info;
                fstat(fd, &info);
                if (info.st_size != size) {
                    if (!ign_size) {
                        ::close(fd);
                        return tpc::Err("Shared memory size doesn't match");
                    } else size = info.st_size;
                }
            }
            data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
            shm_unlink(name.c_str());
            return true;
        }

        void *data;
        std::string name;
        ui size;
        int fd = -1;

        friend class Topic;
    };

    using Shm = std::unique_ptr<SharedMemory>;

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

    using Sem = std::unique_ptr<Semaphore>;

    Sem SemMake(const std::string &name) {
        return std::make_unique<Semaphore>(name);
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

    class SemaphoreRange {
    public:
        explicit SemaphoreRange(ui count) {
            this->count = count;
            sem = (sem_t **) malloc(count * sizeof(sem_t *));
        }

        ~SemaphoreRange() {
            free(sem);
        }

        ui count;
        sem_t **sem;
    };

    using SemRange = std::unique_ptr<SemaphoreRange>;

    SemRange SemRangeMake(ui count) {
        return std::make_unique<SemaphoreRange>(count);
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

class Topic {
public:
    static bool was_interrupted() {
        return tpc::interrupted;
    }

    using TopPtr = std::unique_ptr<Topic>;

    static bool remove(const std::string &name) {
        std::unique_ptr<Topic> t(new Topic(name, 0, 0));
        return t->remove();
    }

    static TopPtr spawn(const std::string &name, ui msg_size, ui msg_count) {
        std::unique_ptr<Topic> t(new Topic(name, msg_size, msg_count));
        if (t->start(false, false, false)) return t;
        else return nullptr;
    }

    static TopPtr spawn(const std::string &name, ui msg_size) {
        std::unique_ptr<Topic> t(new Topic(name, msg_size, 0));
        if (t->start(false, false, true)) return t;
        else return nullptr;
    }

    static TopPtr spawn_create(const std::string &name, ui msg_size, ui msg_count) {
        std::unique_ptr<Topic> t(new Topic(name, msg_size, msg_count));
        if (t->start(true, false, false)) return t;
        else return nullptr;
    }

    static TopPtr spawn(const std::string &name) {
        std::unique_ptr<Topic> t(new Topic(name, 0, 0));
        if (t == nullptr) return nullptr;
        if (t->start(false, true, true)) return t;
        else return nullptr;
    }

    bool pub(const void *msg) {
        DEBUG_MSG("Entered pub in " + name, DF4);
        if (tpc::interrupted) return false;
        auto l = tpc::WriterLock(nlock, WposSRC, wlocks->sem, msg_count);
        if (!l.locked) {
//            if (tpc::interrupted) exit(0);
            return false;
        }
        Wpos = l.pos;
        memcpy(data[Wpos], msg, msg_size);
        return true;
    }

    bool sub(const void *msg) {
        DEBUG_MSG("Entered sub in " + name, DF4);
        if (tpc::interrupted) return false;
        DEBUG_MSG("Reader pos: "+std::to_string(Rpos), DF2);
        auto l = tpc::ReadersLock(rlocks->sem[Rpos], Rcounters[Rpos], wlocks->sem[Rpos]);
        if (!l.locked) {
//            if (tpc::interrupted) exit(0);
            return false;
        }
        memcpy((void*)msg, data[Rpos], msg_size);
        Rpos = (Rpos + 1) % msg_count;
        return true;
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
        full_size = DATA_START + (msg_size + UI_SZ) * msg_count;
        DEBUG_MSG("Full size " << full_size, DF5);
        memory = tpc::ShmMake(name, full_size);
        semCreate = tpc::SemMake(name + "--C");
    }

    ui getWpos() {
        auto l = tpc::Lock(nlock);
        ui pos = *WposSRC;
        return pos;
    }

    bool remove() {
        if (semN != nullptr) semN->remove();
        for (auto i = semW.begin(); i != semW.end(); i++) i->get()->remove();
        for (auto i = semR.begin(); i != semR.end(); i++) i->get()->remove();
        if (memory != nullptr) memory->remove();
        return true;
    }

    bool start(bool create, bool ign_size, bool ign_count) {
        if (steady) return true;
        char *mp, *mpd;
        semN = tpc::SemMake(name + "--n");
        semR.clear();
        semW.clear();
        for (ui i = 0; i < msg_count; i++) semR.push_back(tpc::SemMake(name + "--r" + std::to_string(i)));
        for (ui i = 0; i < msg_count; i++) semW.push_back(tpc::SemMake(name + "--w" + std::to_string(i)));
        bool existed = true;
        if (!semCreate->open_create(1)) return tpc::Err("Unable to create Shmem lock");
        {
            auto l = tpc::Lock(semCreate->sem);
            if (!memory->exists()) {
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
                create_sems();
                Rcounters.clear();
                for (int i = 0; i < msg_count; i++) Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ)));
                for (int i = 0; i < msg_count; i++) (*Rcounters[i]) = 0;
            }
        }
        if (existed) {
            if (!memory->open(true)) return tpc::Err("Topic existed, but errors occured while opening");
            mp = (char *) memory->data;
            mpd = mp + DATA_START;
            auto hdr = (Header *) mp;
            if (msg_size != hdr->msg_size) {
                if (!ign_size) return tpc::Err("Proposed message size doesn't match existing topic message size");
                msg_size = hdr->msg_size;
            }
            if (msg_count != hdr->msg_count) {
                if (!ign_count) return tpc::Err("Proposed message count doesn't match existing topic message count");
                msg_count = hdr->msg_count;
            }
            WposSRC = &(hdr->writer_pos);
            Rpos = 0;
            Rcounters.clear();
            for (int i = 0; i < msg_count; i++) Rcounters.push_back((ui *) (mpd + i * (msg_size + UI_SZ)));
        }
        if (msg_size <= 0) return tpc::Err("Message size should be > 0");
        if (msg_count <= 0) return tpc::Err("Message count should be > 0");
        full_size = memory->size;
        open_sems();
        data.clear();
        for (int i = 0; i < msg_count; i++) data.push_back(mpd + i * (msg_size + UI_SZ) + UI_SZ);
        Rpos = getWpos();
        return true;
    }

    bool create_sems() {
        semN->remove();
        if (!semN->create(1)) return tpc::Err("Can't create W_POS semaphore");
        for (auto i = semR.begin(); i != semR.end(); i++) i->get()->remove();
        for (auto i = semW.begin(); i != semW.end(); i++) i->get()->remove();
        if ((!semR[0]->create(1)) || (!semW[0]->create(0))) return tpc::Err("Can't create W/R semaphore(0)");
        for (int i = 1; i < msg_count; i++)
            if (!semR[i]->create(1) || !semW[i]->create(1)) return tpc::Err("Can't create W/R semaphore(all)");
        return true;
    }

    bool open_sems() {
        if (!semN->open()) return tpc::Err("Can't open W_POS semaphore");
        for (int i = 0; i < msg_count; i++)
            if ((!semR[i]->open()) || (!semW[i]->open())) return tpc::Err("Can't open W/R semaphore");
        rlocks = tpc::SemRangeMake(msg_count);
        wlocks = tpc::SemRangeMake(msg_count);
        for (int i = 0; i < msg_count; i++) {
            rlocks->sem[i] = semR[i]->sem;
            wlocks->sem[i] = semW[i]->sem;
        }
        nlock = semN->sem;
        return true;
    }

    bool steady;
    tpc::Shm memory;
    tpc::Sem semN, semCreate;
    std::vector<tpc::Sem> semW, semR;
    tpc::SemRange wlocks, rlocks;
    sem_t *nlock;
    std::vector<char *> data;
    std::vector<ui *> Rcounters;
    ui Wpos, *WposSRC, Rpos;
    std::string name;
    ui msg_size, msg_count, full_size;
};

template<typename Q, typename A>
class Service {
public:
    using Serv = std::unique_ptr<Service>;

    static Serv create_server(const std::string &name, ui msg_count) {
        Serv s(new Service(name, msg_count, false));
        if (s->working) return s;
        else return nullptr;
    }

    static Serv create_client(const std::string &name, ui msg_count) {
        Serv s(new Service(name, msg_count, true));
        if (s->working) return s;
        else return nullptr;
    }

    static bool remove(const std::string &name){
        return (Topic::remove(name + serv_in_suff) && Topic::remove(name + serv_out_suff));
    }

    struct QueryMessage {
        int pid;
        ui qid;
        bool ackn;
        Q body;
    };
    struct AnswerMessage {
        int pid;
        ui qid;
        bool success;
        A body;
    };
    class Query{
    public:
        bool answer(const A &answer){
            a_msg.pid = q_msg.pid;
            a_msg.qid = q_msg.qid;
            a_msg.success = true;
            a_msg.body = answer;
            DEBUG_MSG("MESSAGE OUT: " + std::to_string(a_msg.qid) + " " + std::to_string(a_msg.pid), DF6);
            return serv->pub_out(a_msg);
        }
        bool requires_answer(){
            return q_msg.ackn && (!answered);
        }
        Q & message(){
            return q_msg.body;
        }
        ~Query(){
            if (requires_answer()){
                a_msg.pid = q_msg.pid;
                a_msg.qid = q_msg.qid;
                a_msg.success = false;
                serv->pub_out(a_msg);
            }
        }
    private:
        Query(const QueryMessage &msg, Service * s){
            serv = s;
            answered = false;
            q_msg = msg;
        }
        bool answered;
        Service * serv;
        friend class Service;
        QueryMessage q_msg;
        AnswerMessage a_msg;
    };

    std::unique_ptr<Query> wait(){
        if (client) return nullptr;
        if (in->sub(&q_msg)){
            DEBUG_MSG("Here query message obtained", DF6);
            std::unique_ptr<Query> t(new Query(q_msg, this));
            return t;
        } else return nullptr;
    }

    A* ask(const Q & msg, bool ackn){
        if (!client) return nullptr;
        q_msg.ackn = ackn;
        q_msg.body = msg;
        if (!in->pub(&q_msg)) return nullptr;
        while (!tpc::interrupted){
            if (out->sub(&a_msg)){
                if (a_msg.pid == q_msg.pid) {
                    if (a_msg.qid == q_msg.qid) {
                        DEBUG_MSG("PIG and QID match", DF6);
                        if (a_msg.success) {
                            q_msg.qid += 1;
                            DEBUG_MSG("Service answered successfully", DF6);
                            return &a_msg.body;
                        } else return nullptr;
                    }
                }
            }
        }
        return nullptr;
    }


private:
    QueryMessage q_msg;
    AnswerMessage a_msg;

    const ui Q_MSG_SIZE = sizeof(QueryMessage);
    const ui A_MSG_SIZE = sizeof(AnswerMessage);
    inline const static std::string serv_in_suff = "--serv-in";
    inline const static std::string serv_out_suff = "--serv-out";

    bool pub_out(const AnswerMessage &answer){
        return out->pub(&answer);
    }

    Service(const std::string &name, ui msg_count, bool is_client) {
        std::string serv_in = name + serv_in_suff;
        std::string serv_out = name + serv_out_suff;
        DEBUG_MSG("Service: will create topics " + serv_in + " and " + serv_out, DF6);
        client = is_client;
        if (msg_count > 0) {
            if (!is_client) {
                in = Topic::spawn_create(serv_in, Q_MSG_SIZE, msg_count);
                out = Topic::spawn_create(serv_out, A_MSG_SIZE, msg_count);
            }
            else{
                in = Topic::spawn(serv_in, Q_MSG_SIZE, msg_count);
                out = Topic::spawn(serv_out, A_MSG_SIZE, msg_count);
            }
        } else {
            in = Topic::spawn(serv_in, Q_MSG_SIZE);
            out = Topic::spawn(serv_out, A_MSG_SIZE);
        }
        q_msg.pid = getpid();
        q_msg.qid = 0;
        working = (nullptr != in && nullptr != out);
    }

    Topic::TopPtr in, out;
    bool client;
    bool working;
};


#endif //PUBSUBCPP2_TOPIC_H

