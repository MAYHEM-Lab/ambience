#pragma once

#include <cstddef>
#include <tos/expected.hpp>
#include <string_view>
#include <type_traits>

namespace tos {
struct error {
    template<class T>
    error(T&& err) : m_model(std::make_unique<model_impl<T>>(std::move(err))) {}

    std::string_view message() const {
        return get_model()->message();
    }

    std::string_view name() const {
        return get_model()->name();
    }

private:
    struct error_model {
        virtual std::string_view name() const;
        virtual std::string_view message() const;
        virtual ~error_model() = default;
    };

    template<class T>
    struct model_impl : error_model {
        model_impl(T&& err) : m_t{std::move(err)} {}
        
        std::string_view name() const override {
            return m_t.name();
        }

        std::string_view message() const override {
            return m_t.message();
        }

        T m_t;
    };

    using raw_storage_t = std::aligned_storage<sizeof(4 * sizeof(void*)), alignof(std::max_align_t)>;
    union {
        raw_storage_t m_inline;
        std::unique_ptr<error_model> m_model;
    };

    error_model* get_model() const {
        return m_model.get();
    }
};

struct const_string_error {
    constexpr const_string_error(std::string_view message) : msg{message} {}

    constexpr std::string_view name() const { 
        return "const_string_error";
    }

    constexpr std::string_view message() const {
        return msg;
    }

    std::string_view msg;
};

template <class T>
using result = expected<T, const_string_error>;
}