#include <tos/condition_variable.hpp>

namespace tos {

void condition_variable::wait(tos::mutex& cond_lock) {
    node n;
    queue.push_back(n);

    cond_lock.unlock();
    n.sem.down();
    cond_lock.lock();
}

void condition_variable::notify_one() {
    if (queue.empty()) {
        return;
    }

    queue.front().sem.up();
    queue.pop_front();
}

void condition_variable::notify_all() {
    while (!queue.empty()) {
        notify_one();
    }
}

} // namespace tos