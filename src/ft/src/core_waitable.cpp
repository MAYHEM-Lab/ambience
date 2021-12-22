#include <tos/core_waitable.hpp>
#include <tos/scheduler.hpp>

namespace tos {
auto core_waitable::add(job& t) -> waiter_handle {
    return m_waiters.push_back(t);
}

job& core_waitable::remove(core_waitable::waiter_handle handle) {
    auto& ret = *handle;
    m_waiters.erase(handle);
    return ret;
}

job& core_waitable::remove(job& t) {
    // job must be in the waitable already, so use unsafe_find
    return remove(m_waiters.unsafe_find(t));
    // return remove(std::find_if(
    //     m_waiters.begin(), m_waiters.end(), [&t](auto& tcb) { return &t == &tcb; }));
}

void core_waitable::signal_all() {
    while (!m_waiters.empty()) {
        signal_one();
    }
}

void core_waitable::signal_one() {
    if (m_waiters.empty())
        return;
    auto& front = m_waiters.front();
    m_waiters.pop_front();
    kern::make_runnable(front);
}

void core_waitable::signal_n(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        signal_one();
    }
}
} // namespace tos