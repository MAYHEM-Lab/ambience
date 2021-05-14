#pragma once

#include <exception>
#include <tos/detail/coro.hpp>
#include <tos/late_constructed.hpp>
#include <variant>

namespace tos {
template<typename T>
class Generator {
public:
    struct GeneratorPromiseType;
    using promise_type = GeneratorPromiseType;
    using promise_coro_handle = std::coroutine_handle<promise_type>;

    Generator() = default;
    Generator(promise_type& promise)
        : m_handle(promise_coro_handle::from_promise(promise)) {
    }

    ~Generator() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

    Generator(Generator&& other) noexcept
        : m_handle(std::exchange(other.m_handle, {})) {
    }

    Generator& operator=(Generator&& other) noexcept {
        if (!this != other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }

        return *this;
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    struct iterator_type;

    iterator_type begin();
    iterator_type end();

private:
    explicit Generator(promise_coro_handle coroutine) noexcept
        : m_handle(coroutine) {
    }

    promise_coro_handle m_handle{};
};

template<typename T>
struct Generator<T>::GeneratorPromiseType {
    const T* value;

    Generator<T> get_return_object() {
        return Generator<T>{promise_coro_handle::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept {
        return {};
    }

    std::suspend_always final_suspend() noexcept {
        return {};
    }

    std::suspend_always yield_value(const T& valRef) noexcept {
        this->value = std::addressof(valRef);
        return {};
    }

    void return_void() {
    }

    template<typename Expression>
    Expression&& await_transform(Expression&& expression) {
        static_assert(sizeof(expression) == 0,
                      "co_await is not supported in coroutines of type generator");
        return std::forward<Expression>(expression);
    }

    void unhandled_exception() {
    }
};

template<typename T>
struct Generator<T>::iterator_type {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    promise_coro_handle m_handle;

    bool operator==(const iterator_type& other) const {
        return m_handle == other;
    }

    bool operator!=(const iterator_type& other) const {
        return m_handle != other.m_handle;
    }

    iterator_type& operator++() {
        m_handle.resume();

        if (m_handle.done()) {
            m_handle = nullptr;
        }

        return *this;
    }

    iterator_type& operator++(int) = delete;

    const T& operator*() const {
        return *m_handle.promise().value;
    }

    const T* operator->() const {
        return std::addressof(operator*());
    }
};

template<typename T>
typename Generator<T>::iterator_type Generator<T>::begin() {
    if (not this->m_handle) {
        return iterator_type{nullptr};
    }

    this->m_handle.resume();

    if (this->m_handle.done()) {
        return iterator_type{nullptr};
    }

    return iterator_type{this->m_handle};
}

template<typename T>
typename Generator<T>::iterator_type Generator<T>::end() {
    return iterator_type{nullptr};
}

} // namespace tos