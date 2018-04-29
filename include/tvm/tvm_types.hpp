//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>
#include <tvm/traits.hpp>

namespace tvm
{
    struct opcode_t
    {
        uint8_t opcode : 7;
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

    template <>
    struct operand_traits<opcode_t>
    {
        static constexpr auto size = 7;
    };

    inline constexpr bool operator==(const opcode_t& a, const opcode_t& b)
    {
        return a.opcode == b.opcode;
    }

    inline constexpr bool operator==(const reg_ind_t<>& a, const reg_ind_t<>& b)
    {
        return a.index == b.index;
    }
}