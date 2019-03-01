//
// Created by fatih on 2/27/19.
//

#pragma once

#include <type_traits>
#include <tos/mutex.hpp>
#include <tos/expected.hpp>

namespace tos
{
    template <class T>
    class future;

    struct promise_info_base
    {
        void wait()
        {
            m_ready.down();
            m_ready.up();
        }

    protected:
        semaphore_base<int8_t> m_ready{0};
    };

    template <class T>
    struct promise_info : public promise_info_base
    {
        T& get()
        {
            wait();
            return *get_ptr();
        }

        void set()
        {
            new (get_ptr()) T();
            m_ready.up();
        }

        void set(const T& v)
        {
            new (get_ptr()) T(v);
            m_ready.up();
        }

        promise_info() = default;

        ~promise_info()
        {
            m_ready.down();
            get_ptr()->~T();
        }

    private:

        T* get_ptr()
        {
            return reinterpret_cast<T *>(&m_storage);
        }

        using store_t = std::aligned_storage_t<sizeof(T), alignof(T)>;

        store_t m_storage;
    };

    template <>
    struct promise_info<void> : public promise_info_base
    {
        void set()
        {
            m_ready.up();
        }

        promise_info() = default;

        ~promise_info()
        {
            m_ready.down();
        }
    };

    template <class T>
    class promise : private promise_info<T>
    {
    public:
        promise() = default;
        promise(const promise&) = delete;
        promise(promise&&) = delete;

        promise&operator=(promise&&) = delete;
        promise&operator=(const promise&) = delete;

        using promise_info<T>::set;

        future<T> get_future();

        ~promise()
        {
            //TODO: signal future if set wasn't called!
        }

    private:

        friend class future<T>;
    };

    template <class T>
    class future
    {
    public:
        void wait()
        {
            m_ptr->wait();
        }

        auto get()
        {
            static_assert(!std::is_same_v<T, void>, "Can't get a future<void>");
            return m_ptr->get();
        }

    private:
        friend class promise<T>;

        future(promise_info<T>& ptr) : m_ptr{&ptr} {}

        promise_info<T>* m_ptr;
    };

    template<class T>
    future<T> promise<T>::get_future() {
        return future<T>(*this);
    }
}
