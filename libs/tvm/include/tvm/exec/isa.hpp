//
// Created by fatih on 4/20/18.
//

#pragma once

#include <algorithm>
#include <array>
#include <tvm/exec/execution.hpp>
#include <tvm/meta.hpp>

namespace tvm {
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

template<class VmT, uint8_t... opcodes, class... Ts>
struct generate_decode_lookup<VmT, list<ins<opcodes, Ts>...>> {
    using ListT = list<ins<opcodes, Ts>...>;

    static constexpr auto value() {
        std::array<executor<VmT>, max_opcode<ListT>::value + 1> lookup{};
        (assign(lookup, opcodes, get_executor<VmT, Ts>()), ...);
        return lookup;
    }

private:
    template<class ArrT, class OpcT, class FunT>
    static constexpr auto assign(ArrT&& t, OpcT&& opc, FunT&& fun) {
        t[opc] = std::forward<FunT>(fun);
    }
};
} // namespace tvm