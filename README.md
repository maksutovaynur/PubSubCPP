# PubSubCPP
C++ Pub-Sub system for message interchange between processes


`static bool Topic::was_interrupted()`
Shows if thread received any interrupting signal (SIGINT, SIGKILL, SIGTERM, SIGQUIT).
Should be noramally used to shut program.

`Topic::TopPtr [= std::unique_ptr<Topic>]`
Safe pointer type for Topic object.

`static bool Topic::remove(const std::string &name)`
Removes existing topic from OS (including shared memory and semaphores)

`static Topic::TopPtr Topic::spawn(const std::string &name, ui msg_size, ui msg_count)`
Creates `Topic` object to manipulate topic.
Returns `nullptr` if topic with this `name` doesn't exist.
Returns also `nullptr` if given `msg_size`, `msg_count` don't match corresponding parameters of existing topic.

`static Topic::TopPtr Topic::spawn(const std::string &name, ui msg_size)`
Creates `Topic` object to manipulate topic.
Returns `nullptr` if topic with this `name` doesn't exist.
Returns also `nullptr` if given `msg_size` doesn't match corresponding parameter of existing topic.


`static TopPtr spawn_create(const std::string &name, ui msg_size, ui msg_count)`
Creates `Topic` object to manipulate topic.
Creates new topic if topic with this `name` doesn't exist.
Returns `nullptr` if given `msg_size`, `msg_count` don't match corresponding parameters of existing topic.

`static TopPtr spawn(const std::string &name)`
Creates `Topic` object to manipulate topic.
Returns `nullptr` if topic with this `name` doesn't exist.

    
`bool Topic::pub(void *msg)`

`bool Topic::sub(void *msg)`

`unsigned long int Topic::get_msg_size()`

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
