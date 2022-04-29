#pragma once

#include <tos/intrusive_list.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/task.hpp>

namespace tos {

class condition_variable {
public:
    void wait(tos::mutex& cond_lock);
    void notify_one();
    void notify_all();

    tos::Task<void> async_wait(tos::mutex& cond_lock) {
        node n;
        queue.push_back(n);

        cond_lock.unlock();
        co_await n.sem;
        co_await cond_lock;
    }


private:
    struct node : tos::list_node<node> {
        tos::semaphore sem{0};
    };

    tos::intrusive_list<node> queue; // Queue of waiting threads
};

} // namespace tos