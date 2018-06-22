//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

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
        using funptr_t = RetT(*)(ArgTs..., void*);

        function_ref(const function_ref& rhs)
            : fun(rhs.fun), data(rhs.data)
        {
        }

        function_ref(funptr_t ptr, void* data)
                : fun(ptr), data(data) {}

        explicit function_ref(funptr_t ptr) : fun(ptr), data(nullptr) {}

        template <class T>
        function_ref(T& func) : fun([](ArgTs&&... args, void* data) {
            auto& foo = *static_cast<const T*>(data);
            foo(std::forward<ArgTs>(args)...);
        }), data(&func) {}

        RetT operator()(ArgTs&&... args)
        {
            return fun(std::forward<ArgTs>(args)..., data);
        }

    private:
        funptr_t fun;
        void* data;
    };
}
