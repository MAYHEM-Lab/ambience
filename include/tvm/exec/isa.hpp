//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/meta.hpp>
#include <tvm/exec/execution.hpp>
#include <algorithm>

namespace tvm
{
    template<class...>
    struct max_opcode;

    template<uint8_t... opcodes, class... Ts>
    struct max_opcode<list<ins<opcodes, Ts>...>> {
        static constexpr auto value = std::max(std::initializer_list<uint8_t>{opcodes...});
    };

    template<class...>
    struct gen_lookup;

    template<class T, size_t sz>
    struct array {
        T data[sz];
    };

    template<uint8_t ... opcodes, class... Ts>
    struct gen_lookup<list<ins<opcodes, Ts>...>> {
        using ListT = list<ins<opcodes, Ts>...>;

        static constexpr auto value() {
            array<executor, max_opcode<ListT>::value + 1> lookup{};
            auto _ = std::initializer_list<int>{(assign(lookup, opcodes, decode_execute<Ts>()), 0)...};
            return lookup;
        }

    private:
        template<class ArrT, class OpcT, class FunT>
        static constexpr auto assign(ArrT &&t, OpcT &&opc, FunT &&fun) {
            t.data[opc] = std::forward<FunT>(fun);
        }
    };
}