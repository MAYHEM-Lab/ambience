//
// Created by fatih on 4/11/18.
//

#pragma once

#include "semaphore.hpp"

namespace tos
{
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
    class mutex
    {
    public:
        /**
         * Initializes a mutex
         *
         * A newly initialized mutex is always in
         * unlocked state.
         */
        mutex() noexcept : m_sem(1) {}

        /**
         * Acquires the mutex.
         *
         * Calling thread will be suspended if the
         * mutex is currently held by another thread.
         */
        void lock() noexcept { m_sem.down(); }

        /**
         * Releases the mutex.
         *
         * If the mutex isn't held by the current
         * thread, the behaviour is undefined.
         *
         * This operation never blocks.
         */
        void unlock() noexcept { m_sem.up(); }

    private:
        tos::semaphore m_sem;
    };

    template <class MutexT>
    class lock_guard
    {
    public:
        explicit lock_guard(MutexT& mtx) noexcept : m_mtx(mtx) { m_mtx.lock(); }
        ~lock_guard() { m_mtx.unlock(); }

        lock_guard(const lock_guard&) = delete;
        lock_guard(lock_guard&&) = delete;

        lock_guard& operator=(lock_guard&&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;

    private:
        MutexT& m_mtx;
    };
}
