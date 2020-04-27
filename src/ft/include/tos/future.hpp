//
// Created by fatih on 2/27/19.
//

#pragma once

#include <tos/debug/assert.hpp>
#include <tos/expected.hpp>
#include <tos/mutex.hpp>
#include <type_traits>

namespace tos {
template<class T>
class future;

struct promise_info_base {
    void wait() {
        m_ready.down();
        m_ready.up();
    }

    [[nodiscard]]
    bool ready() const {
        return get_count(m_ready) != 0;
    }

    uint8_t ref_cnt = 1;

protected:
    semaphore_base<int8_t> m_ready{0};
};

template<class T>
struct promise_info : public promise_info_base {
    T& get() {
        wait();
        return *get_ptr();
    }

    void set() {
        new (get_ptr()) T();
        m_ready.up();
    }

    void set(const T& v) {
        new (get_ptr()) T(v);
        m_ready.up();
    }

    promise_info() = default;

    ~promise_info() {
        m_ready.down();
        std::destroy_at(get_ptr());
    }

private:
    T* get_ptr() {
        return reinterpret_cast<T*>(&m_storage);
    }

    using store_t = std::aligned_storage_t<sizeof(T), alignof(T)>;

    store_t m_storage;
};

template<>
struct promise_info<void> : public promise_info_base {
    void set() {
        m_ready.up();
    }

    void set_isr() {
        m_ready.up_isr();
    }

    promise_info() = default;

    ~promise_info() {
        m_ready.down();
    }
};

template<class T>
class promise {
public:
    promise()
        : m_info(new promise_info<T>) {
    }
    promise(const promise&) = delete;
    promise(promise&& p)
        : m_info(std::exchange(p.m_info, nullptr)) {
    }

    promise& operator=(promise p) {
        std::swap(p.m_info, m_info);
        return *this;
    }

    ~promise() {
        if (!m_info)
            return;

        tos::int_guard ig;

        if (m_info->ref_cnt == 1) {
            delete m_info;
        } else {
            --m_info->ref_cnt;
        }
    }

    void set() {
        m_info->set();
    }

    void set_isr() {
        m_info->set_isr();
    }

    template <class U = T>
    std::enable_if_t<!std::is_same_v<U, void>> set(U&& t) {
        m_info->set(std::forward<U>(t));
    }

    future<T> get_future();

private:
    promise_info<T>* m_info;
    friend class future<T>;
};

template<class T>
class future {
public:
    future() = default;

    void busy_wait() {
        Expects(m_ptr);
        while (!m_ptr->ready());
    }

    void wait() {
        Expects(m_ptr);
        m_ptr->wait();
    }

    auto get() {
        Expects(m_ptr);
        static_assert(!std::is_same_v<T, void>, "Can't get a future<void>");
        return m_ptr->get();
    }

    ~future() {
        if (!m_ptr)
            return;

        tos::int_guard ig;

        if (m_ptr->ref_cnt == 1) {
            delete m_ptr;
        } else {
            --m_ptr->ref_cnt;
        }
    }

    future(future&& f)
        : m_ptr{std::exchange(f.m_ptr, nullptr)} {
    }

    future(const future&) = delete;

    future& operator=(future&& f) {
        std::swap(f.m_ptr, m_ptr);
        return *this;
    }

private:
    friend class promise<T>;

    future(promise_info<T>& ptr)
        : m_ptr{&ptr} {
        tos::int_guard ig;
        m_ptr->ref_cnt++;
    }

    promise_info<T>* m_ptr = nullptr;
};

template<class T>
future<T> promise<T>::get_future() {
    return future<T>(*this->m_info);
}
} // namespace tos
