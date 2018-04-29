//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>
#include <tvm/meta.hpp>

namespace tvm
{
    template <class> struct operand_traits;

    template <class T>
    constexpr uint8_t operand_size_v = operand_traits<T>::size;
}