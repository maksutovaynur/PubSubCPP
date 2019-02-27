# PubSubCPP
C++ Pub-Sub system for message interchange between processes


`static bool was_interrupted() {
    return tpc::interrupted;
}

    using TopPtr = std::unique_ptr<Topic>;

    static bool remove(const std::string &name) {
        auto t = std::make_unique<Topic>(name, 0, 0);
        return t->remove();
    }

    static TopPtr spawn(const std::string &name, ui msg_size, ui msg_count) {
        auto t = std::make_unique<Topic>(name, msg_size, msg_count);
        if (t->start(false, false, false)) return t;
        else return nullptr;
    }

    static TopPtr spawn(const std::string &name, ui msg_size) {
        auto t = std::make_unique<Topic>(name, msg_size, 0);
        if (t->start(false, false, true)) return t;
        else return nullptr;
    }

    static TopPtr spawn_create(const std::string &name, ui msg_size, ui msg_count) {
        auto t = std::make_unique<Topic>(name, msg_size, msg_count);
        if (t->start(true, false, false)) return t;
        else return nullptr;
    }

    static TopPtr spawn(const std::string &name) {
        auto t = std::make_unique<Topic>(name, 0, 0);
        if (t == nullptr) return nullptr;
        if (t->start(false, true, true)) return t;
        else return nullptr;
    }

    bool pub(void *msg) {
        if (tpc::interrupted) return false;
        auto l = tpc::WriterLock(nlock, WposSRC, wlocks->sem, msg_count);
        if (!l.locked){
//            if (tpc::interrupted) exit(0);
            return false;
        }
        Wpos = l.pos;
        memcpy(data[Wpos], msg, msg_size);
        return true;
    }

    bool sub(void *msg) {
        if (tpc::interrupted) return false;
        auto l = tpc::ReadersLock(rlocks->sem[Rpos], Rcounters[Rpos], wlocks->sem[Rpos]);
        if (!l.locked){
//            if (tpc::interrupted) exit(0);
            return false;
        }
        memcpy(msg, data[Rpos], msg_size);
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
`