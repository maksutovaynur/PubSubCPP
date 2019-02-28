# PubSubCPP

C++ Pub-Sub system for message interchange between processes

Create `Topic` object
-----

- `static TopPtr spawn(const std::string &name)`

Creates `Topic` object to manipulate topic.

Returns `nullptr` if topic with this `name` doesn't exist.



- `static Topic::TopPtr Topic::spawn(const std::string &name, ui msg_size)`

Creates `Topic` object to manipulate topic.

Returns `nullptr` if topic with this `name` doesn't exist.

Returns also `nullptr` if given `msg_size` doesn't match corresponding parameter of existing topic.

- `static Topic::TopPtr Topic::spawn(const std::string &name, ui msg_size, ui msg_count)`

Creates `Topic` object to manipulate topic.

Returns `nullptr` if topic with this `name` doesn't exist.

Returns also `nullptr` if given `msg_size`, `msg_count` don't match corresponding parameters of existing topic.


Create and remove topic in OS
-----

- `static TopPtr spawn_create(const std::string &name, ui msg_size, ui msg_count)`

Creates `Topic` object to manipulate topic.

Creates new topic if topic with this `name` doesn't exist.

Returns `nullptr` if given `msg_size`, `msg_count` don't match corresponding parameters of existing topic.

- `static bool Topic::remove(const std::string &name)`

Removes existing topic from OS (including shared memory and semaphores)

Publish and subscribe
-----
    
- `bool Topic::pub(void *msg)`

Publishes given message to a topic. If write wasn't successful, returns `false`.

- `* msg` should have at least `msg_size` of memory allocated. This function will only replace first `msg_size` bytes of memory, allocated with `* msg`.

- `bool Topic::sub(void *msg)`

Takes one message from topic. Blocks current thread until new message, if topic is empty.

Returns `false` if read was unsuccessful (e.g. there was encountered lock error or SIGxxxx was catched by program.



Check `Topic` and system info
-----

- `static bool Topic::was_interrupted()`

Shows if thread received any interrupting signal (SIGINT, SIGKILL, SIGTERM, SIGQUIT).

You should make your program normally shut down after `Topic::was_interrupted()` has `true` value.

- `unsigned long int Topic::get_msg_size()`

Returns `msg_size` of topic.

- `unsigned long int Topic::get_msg_count()`

Returns `msg_count` of topic. Means the maximum count of existing messages in shared memory. 

- `unsigned long int Topic::get_shmem_size()`

Returns the full size of shared memory, allocated for topic, including header and buffer for messages.

- `bool is_ready()`

Returns `true` if topic is ready to publish or subscribe to topic. > Not useful

- `const std::string & Topic::get_name()`

Returns name of topic.

- `Topic::TopPtr [= std::unique_ptr<Topic>]`

Safe pointer type for Topic object.

`
