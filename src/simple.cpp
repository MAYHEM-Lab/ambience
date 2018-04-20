//
// Created by fatih on 4/19/18.
//

#include <tvm/tvm_types.hpp>
#include <tvm/vm_state.hpp>

struct instruction
{
public:
    constexpr instruction(uint8_t op, uint8_t r1, uint8_t r2)
            : m_opcode(op), m_reg1(r1), m_reg2(r2)
    {
    }

    constexpr opcode_t opcode() const
    {
        return { static_cast<uint8_t>(m_opcode) };
    }

    constexpr auto reg1() const
    {
        return m_reg1;
    }

    constexpr auto reg2() const
    {
        return m_reg2;
    }

private:
    uint16_t m_opcode : 7;
    uint16_t m_reg1 : 4;
    uint16_t m_reg2 : 5;
};

static_assert(sizeof(instruction) == sizeof(uint16_t));

using inst_ref = const instruction&;

using executor = void(*)(vm_state*, inst_ref);

constexpr void mov(vm_state* state, inst_ref ins)
{
    state->registers[ins.reg1()] = ins.reg2();
}

constexpr void add(vm_state* state, inst_ref ins)
{
    state->registers[ins.reg1()] += state->registers[ins.reg2()];
}

constexpr void mul(vm_state* state, inst_ref ins)
{
    state->registers[ins.reg1()] *= state->registers[ins.reg2()];
}

constexpr executor get(opcode_t opcode)
{
    switch (opcode.opcode)
    {
        case 0x01: return mov;
        case 0x02: return (executor)add;
        case 0x03: return mul;
        default:
            return nullptr;
    }
}

constexpr instruction instrs[] = {
        {0x01, 0x00, 0x5},
        {0x01, 0x01, 0x10},
        {0x02, 0x00, 0x01},
        {0x03, 0x00, 0x01}
};

template <class T>
constexpr uint32_t eval(const T& instrs)
{
    vm_state state{};

    for (auto& inst : instrs)
    {
        get(inst.opcode())(&state, inst);
    }

    return state.registers[0];
}

constexpr auto res = eval(instrs);
static_assert(res == 336, "");
