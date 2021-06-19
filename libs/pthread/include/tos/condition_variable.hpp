#pragma once

#include <tos/intrusive_list.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos {

class condition_variable {
public:
    void wait(tos::mutex& cond_lock);
    void notify_one();
    void notify_all();

private:
    struct node : tos::list_node<node> {
        tos::semaphore sem{0};
    };

    tos::intrusive_list<node> queue; // Queue of waiting threads
};

} // namespace tos