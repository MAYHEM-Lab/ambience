//
// Created by fatih on 4/11/18.
//

#pragma once

#include <optional>
#include <tos/semaphore.hpp>

namespace tos {
/**
 * This class implements a mutual exclusion
 * primitive.
 *
 * Instances of this type do not implement
 * recursive mutex semantics. A thread can
 * easily deadlock if it tries to lock a mutex
 * it currently holds.
 *
 * + Only one thread may hold a mutex at one time
 */
class mutex : public non_copy_movable {
public:
    /**
     * Initializes a mutex
     *
     * A newly initialized mutex is always in
     * unlocked state.
     */
    mutex() noexcept
        : m_sem(1) {
    }

    /**
     * Acquires the mutex.
     *
     * Calling thread will be suspended if the
     * mutex is currently held by another thread.
     */
    void lock() noexcept {
        m_sem.down();
    }

    bool try_lock(const no_interrupts& = tos::int_guard{}) {
        return try_down_isr(m_sem);
    }

    auto operator co_await() {
        return m_sem.operator co_await();
    }

    /**
     * Releases the mutex.
     *
     * If the mutex isn't held by the current
     * thread, the behaviour is undefined.
     *
     * This operation never blocks.
     */
    void unlock() noexcept {
        m_sem.up();
    }

private:
    tos::semaphore_base<int8_t> m_sem;
};

/**
 * This class implements a recursive mutex abstraction over a regular mutex.
 *
 * The difference from a normal mutex is that the same thread can lock the same mutex
 * multiple times without running into a deadlock.
 *
 * Obviously, this is heavier compared to a raw mutex and should only be used when
 * absolutely necessary.
 */
class recursive_mutex {
public:
    void lock() noexcept {
        if (m_current_holder == tos::this_thread::get_id()) {
            ++m_depth;
            return;
        }
        m_base_mutex.lock();
        m_depth = 1;
        m_current_holder = tos::this_thread::get_id();
    }

    void unlock() noexcept {
        if (--m_depth == 0) {
            m_current_holder = {0};
            m_base_mutex.unlock();
        }
    }

    std::optional<tos::thread_id_t> current_holder() const {
        return m_current_holder == tos::thread_id_t{0}
                   ? std::nullopt
                   : std::make_optional(m_current_holder);
    }

private:
    tos::thread_id_t m_current_holder{0};
    int8_t m_depth{0};
    tos::mutex m_base_mutex;
};

template<class MutexT>
class lock_guard {
public:
    explicit lock_guard(MutexT& mtx) noexcept
        : m_mtx(mtx) {
        m_mtx.lock();
    }
    ~lock_guard() {
        m_mtx.unlock();
    }

    lock_guard(const lock_guard&) = delete;
    lock_guard(lock_guard&&) = delete;

    lock_guard& operator=(lock_guard&&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

private:
    MutexT& m_mtx;
};

struct defer_lock_t {};
constexpr inline defer_lock_t defer_lock;

struct adopt_lock_t {};
constexpr inline adopt_lock_t adopt_lock;

struct try_to_lock_t {};
constexpr inline try_to_lock_t try_to_lock;

template<class MutexT>
class unique_lock {
public:
    using mutex_type = MutexT;

    unique_lock() noexcept
        : m_mtx{nullptr}
        , m_taken{false} {
    }

    explicit unique_lock(MutexT& mtx) noexcept
        : m_mtx{&m_mtx}
        , m_taken{false} {
        lock();
    }

    unique_lock(MutexT& mtx, defer_lock_t) noexcept
        : m_mtx{&mtx}
        , m_taken{false} {
    }

    unique_lock(MutexT& mtx, adopt_lock_t) noexcept
        : m_mtx{&mtx}
        , m_taken{true} {
    }

    unique_lock(MutexT& mtx, try_to_lock_t) noexcept
        : m_mtx{&mtx}
        , m_taken{m_mtx->try_lock()} {
    }

    unique_lock(const unique_lock&) = delete;
    unique_lock& operator=(const unique_lock&) = delete;

    unique_lock(unique_lock&& rhs) noexcept
        : m_mtx(std::exchange(rhs.m_mtx, nullptr))
        , m_taken(std::exchange(rhs.m_taken, false)) {
    }

    unique_lock& operator=(unique_lock&& rhs) noexcept {
        if (m_taken) {
            unlock();
        }

        unique_lock{std::move(rhs)}.swap(*this);

        rhs.m_mtx = nullptr;
        rhs.m_taken = false;

        return *this;
    }

    ~unique_lock() {
        if (m_taken) {
            unlock();
        }
    }

    bool try_lock() noexcept {
        Assert(!owns_lock());
        return m_taken = m_mtx->try_lock();
    }

    void lock() noexcept {
        Assert(!owns_lock());
        m_taken = m_mtx->lock();
    }

    void unlock() noexcept {
        Assert(owns_lock());
        if (m_mtx) {
            m_mtx->unlock();
            m_taken = false;
        }
    }

    [[nodiscard]] bool owns_lock() const noexcept {
        return m_taken;
    }

    MutexT* release() noexcept {
        auto res = m_mtx;
        m_mtx = nullptr;
        m_taken = false;
        return res;
    }

    void swap(unique_lock& rhs) noexcept {
        std::swap(m_mtx, rhs.m_mtx);
        std::swap(m_taken, rhs.m_taken);
    }

    explicit operator bool() const noexcept {
        return owns_lock();
    }

    MutexT* mutex() const noexcept {
        return m_mtx;
    }

private:
    MutexT* m_mtx;
    bool m_taken;
};
} // namespace tos
