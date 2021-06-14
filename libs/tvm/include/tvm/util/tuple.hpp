//
// Created by fatih on 4/29/18.
//

#pragma once

namespace tvm
{
    template <int N, class T>
    struct tuple_node
    {
        T t;
    };

    template <class... T>
    struct tuple
    {

    };
}