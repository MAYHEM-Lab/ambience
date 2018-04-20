#include <iostream>

#include <tvm/meta.hpp>
#include <tvm/vm_state.hpp>
#include <tvm/tvm_types.hpp>
#include <tvm/traits.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/decoding.hpp>
#include <tvm/execution.hpp>
#include <tvm/instructions.hpp>
#include <tvm/isa.hpp>

#include <byteswap.h>
#include <unordered_map>

using ISA = list <ins<0x01, add>, ins<0x02, mov>>;

constexpr executor get_executor(opcode_t c)
{
    constexpr auto lookup = gen_lookup<ISA>::value();
    return lookup[c.opcode];
}

constexpr uint8_t exec_one(vm_state *state, uint32_t instr)
{
    return get_executor(get_opcode(instr))(state, instr);
}

template <class Instrs>
constexpr uint16_t exec(const Instrs& prog)
{
    auto* pos = prog;
    vm_state state{};

    auto load = [&pos]{
        auto p = pos;
        uint32_t res = 0;
        res |= *p++; res <<= 8;
        res |= *p++; res <<= 8;
        res |= *p++; res <<= 8;
        res |= *p++;
        return res;
    };

    pos += exec_one(&state, load());
    pos += exec_one(&state, load());
    pos += exec_one(&state, load());
    return state.registers[0];
}

int main() {
    constexpr uint8_t prog[12] = {
            0x4, 0x0, 0x0, 0x40, // mov %r0, 2
            0x4, 0x20, 0x0, 0x80, // mov %r1, 4
            0x2, 0x2, 0x0, 0x0 // add %r0, %r1
    };

    constexpr auto res = exec(prog);
    std::cout << res << '\n';

    return 0;
}