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
        void* data;
        funptr_t fun;

        RetT operator()(ArgTs&&... args)
        {
            return fun(tos::forward<ArgTs>(args)..., data);
        }
    };
}
