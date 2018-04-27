//
// Created by fatih on 4/27/18.
//
#pragma once

#include <tvm/vm_state.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/decoding.hpp>
#include <ostream>

using printer = uint8_t(*)(std::ostream&, uint32_t);

template <class fun>
constexpr printer print_args()
{
    using T = fun;
    using traits = functor_traits<T>;
    using args_t = tail_t<typename traits::arg_ts>;

    return [](std::ostream& os, uint32_t instr){
        auto len = instruction_len<T>();
        auto shift = (offset_bits<T>() + (4 - len) * 8);
        auto args = decode(args_t{}, instr >> shift);
        os << instr_name_v<fun> << " ";
        print (os, args);
        return len;
    };
}
