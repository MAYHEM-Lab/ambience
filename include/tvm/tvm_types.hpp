//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>
#include <tvm/traits.hpp>

namespace tvm
{

    template <uint8_t sz = 7>
    struct opcode_t
    {
        uint8_t opcode : sz;
    };

    template <uint8_t sz = 4>
    struct reg_ind_t
    {
        uint8_t index : sz;
    };

    template <uint8_t sz>
    struct operand_t
    {
        uint32_t operand : sz;
    };

    template <uint8_t N>
    struct operand_traits<operand_t<N>>
    {
        static constexpr auto size = N;
    };

    template <uint8_t N>
    struct operand_traits<reg_ind_t<N>>
    {
        static constexpr auto size = N;
    };

    template <uint8_t N>
    struct operand_traits<opcode_t<N>>
    {
        static constexpr auto size = N;
    };

    template <uint8_t N>
    inline constexpr bool operator==(const opcode_t<N>& a, const opcode_t<N>& b)
    {
        return a.opcode == b.opcode;
    }

    template <uint8_t N>
    inline constexpr bool operator==(const reg_ind_t<N>& a, const reg_ind_t<N>& b)
    {
        return a.index == b.index;
    }
}