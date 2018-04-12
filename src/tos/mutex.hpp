//
// Created by fatih on 4/11/18.
//

#pragma once

#include "semaphore.hpp"

namespace tos
{
    class mutex
    {
    public:
        mutex() : m_sem(1) {}

        void lock() { m_sem.down(); }
        void unlock() { m_sem.up(); }

    private:
        ft::semaphore m_sem;
    };

    template <class MutexT>
    class lock_guard
    {
    public:
        lock_guard(MutexT& mtx) : m_mtx(mtx) { m_mtx.lock(); }
        ~lock_guard() { m_mtx.unlock(); }

        lock_guard(const lock_guard&) = delete;
        lock_guard(lock_guard&&) = delete;

        lock_guard& operator=(lock_guard&&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;

    private:
        MutexT& m_mtx;
    };
}
