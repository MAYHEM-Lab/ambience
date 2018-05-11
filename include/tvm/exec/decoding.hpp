//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/instr_traits.hpp>
#ifdef TOS
#include <tos/tuple.hpp>
    using tos::tuple;
    using tos::index_sequence;
#else
#include <tuple>
    using std::tuple;
    using std::index_sequence;
#endif

namespace tvm
{
    template<class T, size_t Offset>
    constexpr T decode_one(uint32_t instr) {
        using traits = operand_traits<T>;
        constexpr auto mask = (1ULL << traits::size) - 1U;
        return T{(instr >> Offset) & mask};
    }


    template<class... T, size_t... Is>
    constexpr tuple<T...> decode_impl(uint32_t instr, index_sequence<Is...>) {
        return {decode_one<T, Is>(instr)...};
    }

    template<class... T>
    constexpr auto decode(list<T...>, uint32_t instr) {
        return decode_impl<T...>(instr, typename offsets<list < T...>>::type{});
    }

    template<uint8_t N>
    inline constexpr opcode_t <N> get_opcode(uint32_t instr) {
        return {instr >> (32U - N)};
    }
}