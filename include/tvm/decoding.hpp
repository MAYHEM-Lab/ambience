//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/instr_traits.hpp>
#include <tuple>

template <class T, size_t Offset>
constexpr T decode_one(uint32_t instr)
{
    using traits = operand_traits<T>;
    constexpr auto mask = (1U << traits::size) - 1U;
    return T{ (instr >> Offset) & mask };
}

template <class... T, size_t... Is>
constexpr std::tuple<T...> decode_impl(uint32_t instr, std::index_sequence<Is...>)
{
    return { decode_one<T, Is>(instr)... };
}

template <class... T>
constexpr auto decode(list<T...>, uint32_t instr)
{
    return decode_impl<T...>(instr, typename offsets<list<T...>>::type{});
}

inline constexpr opcode_t get_opcode(uint32_t instr)
{
    return { instr >> 25U };
}
