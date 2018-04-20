//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/vm_state.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/decoding.hpp>
#include <ostream>

using executor = uint8_t(*)(vm_state*, uint32_t);
using printer = uint8_t(*)(std::ostream&, uint32_t);

template <auto fun>
constexpr executor decode_execute()
{
    using T = decltype(fun);
    using traits = function_traits<T>;
    using args_t = tail_t<typename traits::arg_ts>;

    return [](vm_state* vm, uint32_t instr){
        auto len = instruction_len<T>();
        auto shift = (offset_bits<T>() + (4 - len) * 8);
        auto args = decode(args_t{}, instr >> shift);
        std::apply([vm](auto... args){
            fun(vm, std::forward<decltype(args)>(args)...);
        }, args);
        return len;
    };
}

template <auto fun>
constexpr printer print_args()
{
    using T = decltype(fun);
    using traits = function_traits<T>;
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

template <auto fun>
constexpr uint8_t get_len()
{
    using T = decltype(fun);
    return instruction_len<T>();
}
