# PubSubCPP

C++ Pub-Sub system for message interchange between processes, based on `Topic` concept.

Includes also `service` architecture built on top of `Topic`.


Common API
-----
`using ui = unsigned int`

Topic API
-----

#### Create `Topic` object
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


#### Create and remove topic in OS

- `static TopPtr spawn_create(const std::string &name, ui msg_size, ui msg_count)`

Creates `Topic` object to manipulate topic.

Creates new topic if topic with this `name` doesn't exist.

Returns `nullptr` if given `msg_size`, `msg_count` don't match corresponding parameters of existing topic.

- `static bool Topic::remove(const std::string &name)`

Removes existing topic from OS (including shared memory and semaphores)

#### Publish and subscribe
    
- `ui Topic::pub(void *msg)`

Publishes given message to a topic. Returns number of bytes writen from `msg` to topic (`0` means fail).

`* msg` should have at least `msg_size` of memory allocated. This function will only write first `msg_size` bytes of memory, allocated with `* msg`.

- `ui Topic::pub(void *msg, ui size)`

Publishes given message to a topic. Returns number of bytes writen from `msg` to topic (`0` means fail).

`* msg` should have at least `size` of memory allocated. This function will only write first `size` bytes of memory, allocated with `* msg`.

- `ui Topic::sub(void *msg)`

Takes one message from topic. Blocks current thread until new message, if topic is empty.

Returns count of bytes written from topic to `msg`.

Returns `0` if read was unsuccessful (e.g. there was encountered lock error or SIGxxxx was catched by program.



#### Check `Topic` and system info

- `static bool Topic::was_interrupted()`

Shows if thread received any interrupting signal (SIGINT, SIGKILL, SIGTERM, SIGQUIT).

You should make your program normally shut down after `Topic::was_interrupted()` has `true` value.

- `ui int Topic::get_msg_size()`

Returns `msg_size` of topic.

- `ui int Topic::get_msg_count()`

Returns `msg_count` of topic. Means the maximum count of existing messages in shared memory. 

- `ui int Topic::get_shmem_size()`

Returns the full size of shared memory, allocated for topic, including header and buffer for messages.

- `bool is_ready()`

Returns `true` if topic is ready to publish or subscribe to topic. > Not useful

- `const std::string & Topic::get_name()`

Returns name of topic.

- `Topic::TopPtr [= std::unique_ptr<Topic>]`

Safe pointer type for `Topic` object.


Topic example usage
-----

#### Remove topic

    #include <iostream>
    #include "topic.hpp"

    int main(int argc, char** args) {
        std::string name = "/mytopic_name";
        if (Topic::remove(name)) std::cout << "Removed topic " << name << std::endl;
        return 0;
    }
    
#### Create `Topic` object, publish and subscribe

    #include <iostream>
    #include "topic.hpp"
    #include <stdio.h>
    
    const unsigned int MSG_SIZE = 256;
    char msg[MSG_SIZE + 1];
    
    int main() {
        auto t = Topic::spawn_create("/mytopic_name", MSG_SIZE, 10);
        std::cout << "Started topic: " << (bool)(t != nullptr) << std::endl << "Enter 0 to pub, 1 to sub" << std::endl;
        if (t == nullptr) return 0;
        int i;
        std::cin >> i;
        if (i == 0){
            std::string m_base;
            int counter = 0;
            std::cout << "Enter string to pub" << std::endl;
            std::cin >> m_base;
            while (!Topic::was_interrupted()) {
                sprintf(msg, "%s%d", m_base.c_str(), counter++);
                t->pub(msg);
                std::cout << msg << "|end" << std::endl;
                sleep(1);
            }
        }else{
            while (!Topic::was_interrupted()){
                t->sub(msg);
                std::cout << msg << "|end" << std::endl;
            }
        }
        return 0;
    }
    
Service API
-----

#### Create server and client

- `service::util::AsyncServer<size_t size_in, size_t size_out> service::create_async_server<size_t size_in, size_t size_out> 
(std::string & name, ui msg_in_cnt, ui msg_out_cnt)`

Creates `AsyncServer` object. Returns `std::shared_ptr` or `nullptr`(in case of any errors).


- `service::util::SyncClient<size_t size_in, size_t size_out> service::create_sync_client<size_t size_in, size_t size_out> 
(std::string & name)`

Creates `SyncClient` object. Returns `std::shared_ptr` or `nullptr`(in case of any errors).

#### Serve client request

- `std::shared_ptr<Request> AsyncServer::wait_request()`

Returns `Request` object, which contains request data and `answer` method to send response (see below).

Returns `nullptr` if wait wasn't successful.

- `void * Request::data()`

Returns pointer to data, contained in request.

- `ui Request::size()`

Returns data size, contained in request.

- `bool Request::requires_answer()`

Returns `true` if request must be answered (or denied).

Returns `false` if request should not be answered.

- `ui Request::answer(void * data, ui size)`

  `ui Request::answer(void * data)`

  `bool Request::deny(void * data)`

These methods should be use to answer `Request`, that requires answer.

If `deny` is used, client will receive `fail` service status (`0`
from `SyncClient::ask` method that should be handled).

#### Request service from client

- `ui SyncClient::ask(void *request, ui request_size, void *response)`

`request` represents request data. `request_size` means bytes count to write into request.

`response` pointer is used to write service answer into it.

Blocks until service answer. Can return `0`, if:

1) Request wasn't sent
2) Server answered with `deny` method


Returns bytes count, returned by service.

- `ui SyncClient::ask(void * request, void * response)`

Same, but uses `request_size=size_in`

- `bool SyncClient::inform(void *request, ui request_size)`

Same, but doesn't wait for server answer.

Use it if don't want to get any answer, but just inform server.

_If you use this functionality too often, likely tou need Topic object instead of Service_

- `bool SyncClient::inform(void * request)`

Same, but `request_size = size_in`

Service example usage
-----

#### Remove service

    #include <iostream>
    #include "topic.hpp"
    #include <stdio.h>
    
    int main() {
        std::string service_name = "/clap0";
        if (service::remove(service_name)) {
            std::cout << "Service : " << service_name << " removed" << std::endl;
        }
        return 0;
    }

#### Create service server, serve

    #include <iostream>
    #include "topic.hpp"
    #include <stdio.h>
    
    template<typename T>
    struct Question {
        T x = 0;
        T y = 0;
    };
    
    
    int main() {
        std::string service_name = "/clap0";
        auto s = service::create_async_server<8, 4>(service_name, 10, 10);
        if (nullptr == s) {
            std::cout << "Server wasnt created" << std::endl;
        }
        std::cout << " Starting serve" << std::endl;
        while (!tpc::interrupted) {
            auto q = s->wait_request();
            if (nullptr == q) {
                DEBUG_MSG("Query is null_ptr", DF6);
                continue;
            }
            if (sizeof(Question<int>) < q->size()) {
                DEBUG_MSG("Query data size "
                + std::to_string(q->size())
                + " if bigger than expected - "
                + std::to_string(sizeof(Question<int>)), DF6);
                continue;
            }
            auto msg = (Question<int>*) q->data();
            auto result = msg->x + msg->y;
    
            std::cout << "SERVICE: [" << msg->x << ", " << msg->y << "]";
            if (q->requires_answer()) {
                std::cout << " -> " << result;
                q->answer(&result, sizeof(int));
            }
            std::cout << std::endl;
        }
    
        return 0;
    }
    
#### Create service client, ask

    #include <iostream>
    #include "topic.hpp"
    #include <stdio.h>
    
    template<typename T>
    struct Question {
        T x = 0;
        T y = 0;
    };
    
    int main() {
        std::string service_name = "/clap0";
    
        auto s = service::create_sync_client<8, 4>(service_name);
        std::cout << "created service " << service_name << std::endl;
        Question<int> msg;
        while (!tpc::interrupted) {
            std::cout << "x and y > ";
            std::cin >> msg.x >> msg.y;
            if (std::cin.fail()) {
                DEBUG_MSG("Fail while reading !", DF6);
                std::cin.clear();
                if (tpc::interrupted) break;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            int response;
            ui cnt = s->ask(&msg, &response);
            if (cnt > 0)
                std::cout << "Response: " << response
                << std::endl;
            else std::cout << "Response: NULL" << std::endl;
        }
    
        return 0;
    }