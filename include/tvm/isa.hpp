//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/meta.hpp>
#include <algorithm>
#include <tvm/execution.hpp>

template <class...>
struct max_opcode;

template <auto ... opcodes, auto... Ts>
struct max_opcode<list<ins<opcodes, Ts>...>>
{
static constexpr auto value = std::max(opcodes...);
};

template <class...>
struct gen_lookup;

template <auto ... opcodes, auto... Ts>
struct gen_lookup<list<ins<opcodes, Ts>...>>
{
using ListT = list<ins<opcodes, Ts>...>;

static constexpr auto value()
{
    std::array<executor, max_opcode<ListT>::value + 1> lookup{};
    auto assign = [&](auto opc, auto fun)
    {
        lookup[opc] = fun;
    };
    auto _ = std::initializer_list<int>{ (assign(opcodes, decode_execute<Ts>()), 0)... };
    return lookup;
}
};
