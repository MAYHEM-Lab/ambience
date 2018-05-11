//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    template <class RetT, class... ArgTs>
    class function_ref;

    template <class RetT, class... ArgTs>
    class function_ref<RetT(ArgTs...)>
    {
    public:
        using funptr_t = RetT(*)(ArgTs..., void*);

        function_ref() = default;

        explicit function_ref(funptr_t ptr, void* data = nullptr)
                : fun(ptr), data(data) {}

        template <class T>
        function_ref(T& func) : fun([](ArgTs&&... args, void* data) {
            auto& foo = *static_cast<const T*>(data);
            foo(forward<ArgTs>(args)...);
        }), data(&func) {}

        RetT operator()(ArgTs&&... args)
        {
            return fun(tos::forward<ArgTs>(args)..., data);
        }

    private:
        funptr_t fun;
        void* data;
    };
}
