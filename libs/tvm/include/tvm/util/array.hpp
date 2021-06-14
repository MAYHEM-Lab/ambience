//
// Created by fatih on 4/29/18.
//

#pragma once

#include <stddef.h>

namespace tvm
{
    template<class T, size_t sz>
    struct array {
        T data[sz];

        constexpr T* begin() { return data; }
        constexpr const T* begin() const { return data; }

        constexpr T* end() { return data + sz; }
        constexpr const T* end() const { return data + sz; }
    };
}