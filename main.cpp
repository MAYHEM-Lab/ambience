#include <iostream>

#include <tvm/meta.hpp>
#include <tvm/vm_state.hpp>
#include <tvm/tvm_types.hpp>
#include <tvm/traits.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/decoding.hpp>
#include <tvm/execution.hpp>
#include <tvm/instructions.hpp>

#include <byteswap.h>
#include <unordered_map>

struct isa
: ins<0x01, add>,
  ins<0x02, mov>
{};

constexpr executor get_executor(opcode_t c)
{
    switch (c.opcode)
    {
        case 0x01: return decode_execute<add>();
        case 0x02: return decode_execute<mov>();
        default:
            return nullptr;
    }
}

uint8_t exec_one(vm_state *state, uint32_t instr)
{
    return get_executor(get_opcode(instr))(state, instr);
}

template <class Instrs>
void exec(const Instrs& prog)
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
    std::cout << state.registers[0] << '\n';
}

int main() {
    uint8_t prog[12] = {
            0x4, 0x0, 0x0, 0x40, // mov %r0, 2
            0x4, 0x20, 0x0, 0x80, // mov %r1, 4
            0x2, 0x2, 0x0, 0x0 // add %r0, %r1
    };

    exec(prog);

    return 0;
}