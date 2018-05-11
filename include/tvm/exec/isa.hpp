//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/meta.hpp>
#include <tvm/exec/execution.hpp>
#include <tvm/util/array.hpp>
#ifdef TOS
#include <tos/algorithm.hpp>
#else
#include <algorithm>
#endif

namespace tvm
{
    template<class...>
    struct max_opcode;

    /**
     * This metafunction calculates the maximum opcode from an ISA description list
     * @tparam list<Instructions...> list of instructions
     */
    template<uint8_t... opcodes, class... Ts>
    struct max_opcode<list<ins<opcodes, Ts>...>> {
#ifndef TOS
        static constexpr auto value = std::max(std::initializer_list<uint8_t>{opcodes...});
#else
        static constexpr uint8_t ops[] = { opcodes... };
        static constexpr auto value = tos::max_range(ops, ops + sizeof...(opcodes));
#endif
    };

    template<class...>
    struct generate_decode_lookup;

    template<class VmT, uint8_t ... opcodes, class... Ts>
    struct generate_decode_lookup<VmT, list<ins<opcodes, Ts>...>> {
        using ListT = list<ins<opcodes, Ts>...>;

        static constexpr auto value() {
            tvm::array<executor<VmT>, max_opcode<ListT>::value + 1> lookup{};
            int _[] {(assign(lookup, opcodes, get_executor<VmT, Ts>()), 0)...};
            return lookup;
        }

    private:
        template<class ArrT, class OpcT, class FunT>
        static constexpr auto assign(ArrT &&t, OpcT &&opc, FunT &&fun) {
            t.data[opc] = forward<FunT>(fun);
        }
    };
}