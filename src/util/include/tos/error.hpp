#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <tos/concepts.hpp>
#include <type_traits>

namespace tos {
template<class T>
concept MemberError = requires(const T& t) {
    {t.name()};
    {t.message()};
};

template<class T>
concept FreeError = requires(const T& t) {
    name(t);
    message(t);
};

template<class T>
concept Error = MemberError<T> || FreeError<T>;

template<Error T>
struct error_traits;

template<FreeError T>
struct error_traits<T> {
    static constexpr std::string_view get_name(const T& t) {
        return name(t);
    }

    static constexpr std::string_view get_message(const T& t) {
        return message(t);
    }
};

template<MemberError T>
struct error_traits<T> {
    static constexpr std::string_view get_name(const T& t) {
        return t.name();
    }

    static constexpr std::string_view get_message(const T& t) {
        return t.message();
    }
};

struct any_error {
    static inline constexpr auto sbo_size = 16;

public:
    template<Error T>
        any_error(T&& err) requires(
            !std::same_as<std::remove_const_t<std::remove_reference_t<T>>, any_error>) &&
        (sizeof(T) < sbo_size) && (std::is_trivially_move_constructible_v<T>) {
        using model_type = model_impl<std::remove_const_t<std::remove_reference_t<T>>>;
        sbo_init<model_type>(std::forward<T>(err));
    }

    template<Error T>
    any_error(T&& err) requires(
        !std::same_as<std::remove_const_t<std::remove_reference_t<T>>, any_error>)
        : m_model(std::make_unique<
                  model_impl<std::remove_const_t<std::remove_reference_t<T>>>>(
              std::forward<T>(err))) {
        mark_sbo(false);
    }


    any_error(any_error&& err) {
        if (err.is_sbo()) {
            memcpy(&m_sbo, &err.m_sbo, sizeof m_sbo);
            return;
        }
        m_model = std::move(err.m_model);
        mark_sbo(false);
    }

    std::string_view message() const {
        return get_model()->message();
    }

    std::string_view name() const {
        return get_model()->name();
    }

    ~any_error() {
        if (is_sbo()) {
            std::destroy_at(get_sbo_model());
            return;
        }
        std::destroy_at(&m_model);
    }

private:
    struct error_model {
        virtual std::string_view name() const = 0;
        virtual std::string_view message() const = 0;
        virtual ~error_model() = default;
    };

    template<Error T>
    struct model_impl
        : error_model
        , T {
        template<class U>
        model_impl(U&& err)
            : T{std::forward<U>(err)} {
        }

        std::string_view name() const override {
            return error_traits<T>::get_name(static_cast<const T&>(*this));
        }

        std::string_view message() const override {
            return error_traits<T>::get_message(static_cast<const T&>(*this));
        }
    };

    union {
        std::unique_ptr<error_model> m_model;
        std::aligned_storage_t<sbo_size, alignof(void*)> m_sbo;
    };

    template<class T, class U>
    void sbo_init(U&& val) {
        new (&m_sbo) T(std::forward<U>(val));
        mark_sbo(true);
    }

    void mark_sbo(bool val) {
        auto last_byte = (char*)&m_sbo + sbo_size - 1;
        *last_byte = val;
    }

    // Returns whether the current error is stored using the small buffer optimization.
    bool is_sbo() const {
        auto last_byte = (char*)&m_sbo + sbo_size - 1;
        return *last_byte;
    }

    const error_model* get_sbo_model() const {
        return reinterpret_cast<const error_model*>(&m_sbo);
    }

    const error_model* get_model() const {
        if (is_sbo()) {
            return get_sbo_model();
        }
        return m_model.get();
    }
};

struct const_string_error {
    constexpr const_string_error(std::string_view message)
        : msg{message} {
    }

    constexpr std::string_view name() const {
        return "const_string_error";
    }

    constexpr std::string_view message() const {
        return msg;
    }

    std::string_view msg;
};
} // namespace tos