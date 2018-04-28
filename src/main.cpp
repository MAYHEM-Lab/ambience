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

#include <unordered_map>
#include <fstream>
#include <vector>

using ISA = list <ins<0x01, add>, ins<0x02, movi>, ins<0x03, exit_ins>>;

constexpr tvm::executor get_executor(opcode_t c)
{
    constexpr auto lookup = tvm::gen_lookup<ISA>::value();
    return lookup.data[c.opcode];
}

constexpr uint8_t exec_one(vm_state *state, uint32_t instr)
{
    return get_executor(get_opcode(instr))(state, instr);
}

constexpr auto load(const uint8_t* p) {
    uint32_t res = 0;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++;
    return res;
};

template <class Instrs>
constexpr uint16_t exec(const Instrs& prog)
{
    auto* pos = prog;
    vm_state state{};

    while (state.registers[15] != 0xDEAD)
    {
        pos += exec_one(&state, load(pos));
    }
    return state.registers[0];
}

int main() {
    std::ifstream in("a.out", std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    constexpr uint8_t prog[14] = {
            0x04, 0x00, 0x00, 0x40, // mov %r0, 2
            0x04, 0x20, 0x00, 0x80, // mov %r1, 4
            0x02, 0x02,             // add %r0, %r1
            0x06, 0x00, 0x00, 0x00  // exit
    };

    auto res = exec(contents.data());
    std::cout << res << '\n';

    return 0;
}