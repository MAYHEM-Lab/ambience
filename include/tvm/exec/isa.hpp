//
// Created by fatih on 4/20/18.
//

#pragma once

#include <tvm/meta.hpp>
#include <tvm/exec/execution.hpp>
#include <tvm/util/array.hpp>
#include <algorithm>

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
        static constexpr auto value = std::max(std::initializer_list<uint8_t>{opcodes...});
    };

    template<class...>
    struct generate_decode_lookup;

    template<uint8_t ... opcodes, class... Ts>
    struct generate_decode_lookup<list<ins<opcodes, Ts>...>> {
        using ListT = list<ins<opcodes, Ts>...>;

        static constexpr auto value() {
            tvm::array<executor, max_opcode<ListT>::value + 1> lookup{};
            auto _ = std::initializer_list<int>{(assign(lookup, opcodes, get_executor<Ts>()), 0)...};
            return lookup;
        }

    private:
        template<class ArrT, class OpcT, class FunT>
        static constexpr auto assign(ArrT &&t, OpcT &&opc, FunT &&fun) {
            t.data[opc] = std::forward<FunT>(fun);
        }
    };
}