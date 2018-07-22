//
// Created by fatih on 7/22/18.
//

#pragma once

namespace tos
{
    class res_base
    {
    public:
        res_base* next;
        using clean_fn = void(*)(res_base*);
        clean_fn cleanup;
        friend void unwind(res_base* base);
    };

    template <class T>
    class res : public res_base, public T
    {
    public:
        res(T&& t) : T(std::move(t)) {
            cleanup = [](res_base* x){
                auto self = static_cast<res<T>*>(x);
                tos::std::destroy_at((T*)self);
            };
        }
    };

    inline void unwind(res_base* base)
    {
        while (base)
        {
            base->cleanup(base);
            base = base->next;
        }
    }
}
