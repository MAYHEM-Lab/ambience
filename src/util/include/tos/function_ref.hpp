//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/debug/panic.hpp>
#include <tos/utility.hpp>

namespace tos
{
    template <class RetT, class... ArgTs>
    class function_ref;

    /**
     * This type implements a non owning reference to a
     * callable object, including raw function pointers.
     *
     * Usage:
     *     auto fr = function_ref<void()>([]{});
     *
     *     auto lambda = [&](int x) -> std::string { ... };
     *     auto fr = function_ref<std::string(int)>(lambda);
     *
     * When used with stateful function objects, it's
     * important to note that the constructor takes an
     * l-value reference, which means we cannot pass
     * temporaries to it.
     *
     * The converting constructor from a raw function
     * pointer is explicit to avoid unwanted constructions.
     */
    template <class RetT, class... ArgTs>
    class function_ref<RetT(ArgTs...)>
    {
    public:
        using internal_funptr_t = RetT(*)(ArgTs..., void*);

        function_ref(const function_ref& rhs) = default;

        function_ref(internal_funptr_t ptr, void* data)
                : m_fun(ptr), m_data(data) {
            if (m_fun == nullptr)
            {
                debug::panic("Function cannot be null!");
            }
        }

        explicit function_ref(internal_funptr_t ptr) : function_ref(ptr, nullptr) {}

        template <class T, std::enable_if_t<!std::is_same_v<T, function_ref>>* = nullptr>
        explicit function_ref(T& func) : m_fun([](ArgTs... args, void* data) -> RetT {
            static_assert(!std::is_const_v<T>, "Function cannot be a temporary!");
            auto& actual_fun = *static_cast<T*>(data);
            return actual_fun(std::forward<ArgTs>(args)...);
        }), m_data(&func) {}

        template <class... CallArgTs>
        RetT operator()(CallArgTs&&... args)
        {
            return m_fun(std::forward<CallArgTs>(args)..., m_data);
        }

        template <class... CallArgTs>
        RetT operator()(CallArgTs&&... args) const
        {
            return m_fun(std::forward<CallArgTs>(args)..., m_data);
        }

    private:
        internal_funptr_t m_fun;
        void* m_data;
    };
}
