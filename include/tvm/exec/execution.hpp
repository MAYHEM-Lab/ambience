//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/vm_state.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/exec/decoding.hpp>
#include <tvm/vm_traits.hpp>

namespace tvm
{
    using executor = uint8_t(*)(vm_state*, uint32_t);

    template <class T>
    constexpr uint8_t executor_impl(vm_state* vm, uint32_t instr)
    {
        using vm_traits = vm_traits<>;
        using traits = functor_traits<T>;
        using args_t = tail_t<typename traits::arg_ts>;
        auto len = instruction_len<T, opcode_len_v<>>();
        auto shift = (offset_bits<T, opcode_len_v<>>() + (4 - len) * 8);
        auto args = decode(args_t{}, instr >> shift);
        apply(T{}, vm, args);
        return len;
    }

    template <class fun>
    constexpr executor decode_execute()
    {
        using T = fun;
        return &executor_impl<T>;
    }

    template <class fun>
    constexpr uint8_t get_len()
    {
        using T = fun;
        return instruction_len<T>();
    }
}