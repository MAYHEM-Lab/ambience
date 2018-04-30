//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>
#include <tvm/meta.hpp>

namespace tvm
{
    template <class> struct operand_traits;

    /**
     * This trait returns the size of an operand type in bits
     *
     * For example, register indices, immediates or even the opcode
     *
     * @tparam T type of the operand
     */
    template <class T>
    constexpr uint8_t operand_size_v = operand_traits<T>::size;
}