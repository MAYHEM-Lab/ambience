//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tuple>

namespace tos {
    using std::tuple;

    static_assert(sizeof(int) == sizeof(tuple<int>), "tuple shouldn't have overhead!");
    static_assert(sizeof(void*) == sizeof(tuple<void*>), "tuple shouldn't have overhead!");
}


