#pragma once

#include <tos/condition_variable.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos {

class shared_mutex {
private:
    int readers = 0, writers = 0;
    tos::mutex mutex;
    tos::condition_variable cv;

public:
    shared_mutex() = default;
    ~shared_mutex() = default;

    // Reader
    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();

    // Writer
    void lock();
    bool try_lock();
    void unlock();

    // Pthread
    void pthread_unlock();
};

} // namespace tos