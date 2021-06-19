#include <tos/shared_mutex.hpp>

namespace tos {

void shared_mutex::lock_shared() {
    tos::lock_guard lock(mutex);
    while (writers) {
        cv.wait(mutex);
    }

    readers++;
}

bool shared_mutex::try_lock_shared() {
    tos::lock_guard lock(mutex);

    if (!writers) {
        readers++;
        return true;
    }

    return false;
}

void shared_mutex::unlock_shared() {
    tos::lock_guard lock(mutex);

    if (readers) {
        readers--;
        cv.notify_all();
    }
}

void shared_mutex::lock() {
    tos::lock_guard lock(mutex);

    // Writer must wait until there are no readers or writers
    while (readers || writers) {
        cv.wait(mutex);
    }

    writers++;
}

bool shared_mutex::try_lock() {
    tos::lock_guard lock(mutex);

    if (!readers && !writers) {
        writers++;
        return true;
    }

    return false;
}

void shared_mutex::unlock() {
    tos::lock_guard lock(mutex);

    if (writers) {
        writers--;
        cv.notify_all();
    }
}

void shared_mutex::pthread_unlock() {
    readers ? unlock_shared() : unlock();
}

} // namespace tos