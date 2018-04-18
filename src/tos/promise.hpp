//
// Created by fatih on 4/16/18.
//

#pragma once

namespace tos
{
    template <class T>
    class promise
    {
    public:
        template <class FunT>
        promise(FunT&& fun)
        {
        }

        void wait();
        T& get();

    private:

    };
}