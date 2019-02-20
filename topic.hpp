#include <memory>

//
// Created by aynur on 2/19/19.
//
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <memory>
#include <vector>

#ifndef PUBSUBCPP_TOPIC_H
#define PUBSUBCPP_TOPIC_H


#define _enable_std_make_unique

namespace topic{
    using ui = unsigned int;

    class SharedMemory{
    public:
        SharedMemory(const std::string &name, ui size){
            this->name = name;
            this->fd = -1;
            this->size = size;
            this->data = MAP_FAILED;
        }
        bool exists(){
            if (fd >= 0) return true;
            fd = shm_open(name.c_str(), O_RDWR, 0777);
            if (fd < 0) return false;
            ::close(fd);
            return true;
        }
        bool create(){
            fd = shm_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0777);
            if (fd < 0) return false;
            ftruncate(fd, size);
            ::close(fd);
            return true;
        }
        bool open(bool ign_size){
            if (MAP_FAILED != data) return true;
            if (!exists()) return false;
            {
                struct stat info;
                fstat(fd, &info);
                if (info.st_size != size) {
                    if (!ign_size) return false;
                    else size = (ui)info.st_size;
                }
            }
            fd = shm_open(name.c_str(), O_RDWR, 0777);
            if (fd < 0) return false;
            data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            return MAP_FAILED != data;
        }
        bool is_open(){
            return MAP_FAILED != data;
        }
        bool close(){
            if (!is_open()) return true;
            munmap(data, size);
            ::close(fd);
            data = MAP_FAILED;
            fd = -1;
            return true;
        }
        bool remove(){
            shm_unlink(name.c_str());
            return true;
        }
        void* data;
        std::string name;
        ui size;
        int fd = -1;
        friend class Topic;
    };
    using Shm = std::unique_ptr<SharedMemory>;
    Shm ShmMake(const std::string &name, ui size){
        return std::make_unique<SharedMemory>(name, size);
    }

    class Semaphore{
    public:
        Semaphore(const std::string &name){
            this->name = name;
            this->sem = SEM_FAILED;
        }
        bool exists(){
            if (sem != SEM_FAILED) return true;
            sem = sem_open(name.c_str(), O_RDWR);
            if (SEM_FAILED == sem) return false;
            sem_close(sem);
            return true;
        }
        bool create(int val){
            sem = sem_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0777, val);
            if (SEM_FAILED == sem) return false;
            sem_close(sem);
            return true;
        }
        bool open(){
            if (SEM_FAILED != sem) return true;
            if (!exists()) return false;
            sem = sem_open(name.c_str(), O_RDWR);
            return MAP_FAILED != sem;
        }
        bool is_open(){
            return SEM_FAILED != sem;
        }
        bool close(){
            if (!is_open()) return true;
            sem_close(sem);
            sem = SEM_FAILED;
            return true;
        }
        bool remove(){
            sem_unlink(name.c_str());
            return true;
        }
        sem_t *sem;
        std::string name;
    };
    using Sem = std::unique_ptr<Semaphore>;
    Sem SemMake(const std::string &name){
        return std::make_unique<Semaphore>(name);
    }

    class Lock{
    public:
        Lock(sem_t *sem){
            this->sem = sem;
            sem_wait(sem);
        }
        ~Lock(){
            sem_post(sem);
        }
        sem_t *sem;
    };

    class ReadersLock{
    public:
        ReadersLock(sem_t *sem, ui *counter, sem_t *cond){
            this->counter = counter;
            this->cond = cond;
            this->sem = sem;
            auto l = Lock(sem);
            if (1 == ++(*counter)) sem_wait(cond);
        }
        ~ReadersLock(){
            auto l = Lock(sem);
            if (0 == --(*counter)) sem_post(cond);
        }
        sem_t *sem, *cond;
        ui *counter;
    };

    class WriterLock{
    public:
        WriterLock(sem_t *sem, ui *counter, sem_t **lim, ui lim_count){
            this->lim = lim;
            auto l = Lock(sem);
            pos = (*counter) ++;
            *counter %= lim_count;
            sem_wait(lim[*counter]);
        }
        ~WriterLock(){
            sem_post(lim[pos]);
        }
        sem_t **lim;
        ui pos;
    };

    class Topic{
    public:
        using Top = std::unique_ptr<Topic>;
        static Top spawn(const std::string &name, ui msg_size, ui msg_count){
            auto t = std::make_unique<Topic> (name, msg_size, msg_count);
            t->start(false, false, false);
            return t;
        }
        static Top spawn(const std::string &name, ui msg_size){
            auto t = std::make_unique<Topic> (name, msg_size, 0);
            t->start(false, false, true);
            return t;
        }
        static Top spawn_create(const std::string &name, ui msg_size, ui msg_count){
            auto t = std::make_unique<Topic> (name, msg_size, msg_count);
            t->start(true, false, false);
            return t;
        }
        static Top spawn(const std::string &name){
            auto t = std::make_unique<Topic> (name, 0, 0);
            t->start(false, true, true);
            return t;
        }
        void pub(void *msg){

        }
        void sub(void *msg){

        }
        ui get_msg_size(){
            return msg_size;
        }
        ui get_msg_count(){
            return msg_count;
        }
        ui get_shmem_size(){
            return full_size;
        }
        ui is_ready(){
            return steady;
        }
    private:
        Topic(const std::string &name, ui msg_size, ui msg_count){
            this->name = name;
            this->msg_size = msg_size;
            this->msg_count = msg_count;
            full_size = DATA_START + (msg_size + UI_SZ) * msg_count;
            memory = ShmMake(name, full_size);
        }
        bool start(bool create, bool ign_size, bool ign_count){
            if (steady) return true;
            void * data;
            if (!memory->exists()){
                if (!create) return false;
                if (!memory->create()) return false;
                if (!memory->open(false)) return false;
                data = memory->data;
                auto hdr = (Header *) data;
                hdr->msg_count = msg_count;
                hdr->msg_size = msg_size;
                hdr->writer_pos = 0;
                WposSRC = &(hdr->writer_pos);
                Rpos = 0;
            }else{
                if (!memory->open(true)) return false;
                data = memory->data;
                auto hdr = (Header *) data;
                if (msg_size != hdr->msg_size) {
                    if (!ign_size) return false;
                    msg_size = hdr->msg_size;
                }
                if (msg_count != hdr->msg_count){
                    if (!ign_count) return false;
                    msg_count = hdr->msg_count;
                }
                WposSRC = &(hdr->writer_pos);
                Rpos = 0;
            }
            full_size = memory->size;
            semN = SemMake(name + "--n"); semN->remove(); if (!semN->create(0)) return false;
            semR.clear();semW.clear();
            for (int i = 0; i < msg_count; i++){
                semR.push_back(SemMake(name + "--r" + std::to_string(i)));
                semW.push_back(SemMake(name + "--w" + std::to_string(i)));
            }
            if (!semR[0]->create(1) || !semW[0]->create(0)) return false;
            for (int i = 1; i < msg_count; i++){
                semR[i]->remove(); semW[i]->remove();
                if (!semR[i]->create(1) || !semW[i]->create(1)) return false;
            }
            for (int i = 0; i < msg_count; i++){
                if (!semR[i]->open() || !semW[i]->open()) return false;
            }
            return true;
        }
        bool stop(){
            return true; // TODO: Здесь пока ничего нет
        }
        bool steady;
        Shm memory;
        Sem semN;
        std::vector<Sem> semW, semR;
        ui Wpos, *WposSRC, Rpos;

        struct Header{
            ui msg_size;
            ui msg_count;
            ui writer_pos;
        };

        static const ui DATA_START = 32;
        static const ui UI_SZ = sizeof(ui);

        std::string name;
        ui msg_size, msg_count;
        ui full_size;
    };
}

#endif //PUBSUBCPP2_TOPIC_H
