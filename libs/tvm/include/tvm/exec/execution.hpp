//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/instr_traits.hpp>
#include <tvm/exec/decoding.hpp>
#include <tvm/vm_traits.hpp>

namespace tvm
{
    template <class VmT>
    using executor = uint8_t(*)(VmT*, uint32_t);

    template <class VmT, class T>
    constexpr uint8_t executor_impl(VmT* vm, uint32_t instr)
    {
        using traits = functor_traits<T>;
        using args_t = tail_t<typename traits::arg_ts>;

        auto len = instruction_len<T, opcode_len_v<>>();
        auto shift = (offset_bits<T, opcode_len_v<>>() + (4 - len) * 8);

        auto args = decode(args_t{}, instr >> shift);

        apply(T{}, vm, args);
        return len;
    }

    template <class VmT, class T>
    constexpr executor<VmT> get_executor()
    {
        return &executor_impl<VmT, T>;
    }
}