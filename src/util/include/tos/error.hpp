#pragma once

#include <cstring>
#include <ctti/nameof.hpp>
#include <magic_enum.hpp>
#include <memory>
#include <string_view>
#include <tos/concepts.hpp>
#include <tos/utility.hpp>
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

template<Enumeration T>
struct enum_error {
    enum_error(T t)
        : m_val{t} {
    }

    T m_val;

    constexpr std::string_view name() const {
        return ctti::nameof_v<T>;
    }

    constexpr std::string_view message() const {
        return magic_enum::enum_name(m_val);
    }
};

namespace detail {
template<class T>
concept ImplicitEnumError = requires(const T& t) {
    {tos_is_implicit_error_enum(t)};
};

#define TOS_ERROR_ENUM(type)                                 \
    constexpr bool tos_is_implicit_error_enum(const type&) { \
        return true;                                         \
    }
} // namespace detail

struct any_error {
    static inline constexpr auto sbo_size = 3 * sizeof(void*);

public:
    template<detail::ImplicitEnumError T>
    any_error(T val)
        : any_error(enum_error<T>(val)) {
    }

    template<Error T>
    any_error(T&& err) requires(
        !std::same_as<std::remove_const_t<std::remove_reference_t<T>>, any_error>) {
        using error_type = std::remove_const_t<std::remove_reference_t<T>>;

        m_vtbl = &implement<error_type>::tbl;

        if constexpr (implement<error_type>::tbl.is_sbo) {
            sbo_init<error_type>(std::forward<T>(err));
        } else {
            m_heap_ptr = new error_type(std::forward<T>(err));
        }
    }

    any_error(any_error&& err) {
        memcpy(static_cast<void*>(this), &err, sizeof *this);
        if (is_sbo()) {
            return;
        }
        err.m_heap_ptr = nullptr;
    }

    std::string_view message() const {
        return m_vtbl->message(get_model());
    }

    std::string_view name() const {
        return m_vtbl->name(get_model());
    }

    ~any_error() {
        if (!get_model()) {
            return;
        }
        m_vtbl->dtor(get_model(), !is_sbo());
    }

    template<class T>
    friend const T* error_cast(const any_error& err) {
        if (err.m_vtbl == &implement<T>::tbl) {
            return static_cast<const T*>(err.get_model());
        }
        return nullptr;
    }

private:
    struct vtbl {
        bool is_sbo;
        std::string_view (*name)(const void*);
        std::string_view (*message)(const void*);
        void (*dtor)(const void*, bool del);
    };

    template<class T>
    struct implement {
        static std::string_view name(const void* ptr) {
            return error_traits<T>::get_name(*static_cast<const T*>(ptr));
        }

        static std::string_view message(const void* ptr) {
            return error_traits<T>::get_message(*static_cast<const T*>(ptr));
        }

        static void dtor(const void* ptr, bool del) {
            if (del) {
                delete static_cast<const T*>(ptr);
                return;
            }
            std::destroy_at(static_cast<const T*>(ptr));
        }

        static constexpr inline vtbl tbl{
            .is_sbo = sizeof(T) <= sbo_size && std::is_trivially_move_constructible_v<T>,
            .name = &name,
            .message = &message,
            .dtor = &dtor,
        };
    };

    const vtbl* m_vtbl;
    union {
        void* m_heap_ptr;
        std::aligned_storage_t<sbo_size, alignof(void*)> m_sbo;
    };

    template<class T, class U>
    void sbo_init(U&& val) {
        new (&m_sbo) T(std::forward<U>(val));
    }

    // Returns whether the current error is stored using the small buffer optimization.
    bool is_sbo() const {
        return m_vtbl->is_sbo;
    }

    const void* get_sbo_model() const {
        return &m_sbo;
    }

    const void* get_model() const {
        if (is_sbo()) {
            return get_sbo_model();
        }
        return m_heap_ptr;
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
