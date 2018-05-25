//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include "vm_state.hpp"
#include <tvm/tvm_types.hpp>
#include <tvm/instr_traits.hpp>

struct add
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<> r1, tvm::reg_ind_t<> r2)
    {
        state->registers[r1.index] += state->registers[r2.index];
    }
};

struct movi
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<> r1, tvm::operand_t<16> op)
    {
        state->registers[r1.index] = (uint16_t)op.operand;
    }
};

struct movr
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<4> r1, tvm::reg_ind_t<4> r2)
    {
        state->registers[r1.index] = state->registers[r2.index];
    }
};

struct jump
{
    constexpr void operator()(svm::vm_state* state, tvm::address_t<16> abs)
    {
        state->pc = abs.addr - 3;
    }
};

struct jumpi
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<4> reg)
    {
        state->pc = state->registers[reg.index] - 2;
    }
};

struct branch_if_eq
{
    constexpr void operator()(svm::vm_state* state,
            tvm::reg_ind_t<4> a, tvm::reg_ind_t<4> b,
            tvm::address_t<16> addr)
    {
        if (state->registers[a.index] == state->registers[b.index])
        {
            state->pc = addr.addr - 4;
        }
    }
};

struct exit_ins
{
    constexpr void operator()(svm::vm_state* state)
    {
        state->registers[14] = 0xDEAD;
    }
};

struct syscall
{
    constexpr void operator()(svm::vm_state* state)
    {
        // syscall id is in r0
        auto sys_index = state->registers[0];
    }
};

struct push
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<4> reg)
    {
        if (state->stack_cur == state->stack_end)
        {
            state->registers[14] = 0xDEAD;
            return;
        }

        *state->stack_cur++ = reg.index == 15 ? state->pc : state->registers[reg.index];
    }
};

struct pop
{
    constexpr void operator()(svm::vm_state* state, tvm::reg_ind_t<4> reg)
    {
        if (state->stack_cur == state->stack_begin)
        {
            state->registers[14] = 0xDEAD;
            return;
        }
        state->registers[reg.index] = *--state->stack_cur;
    }
};

struct call
{
    constexpr void operator()(svm::vm_state* state, tvm::address_t<16> abs)
    {
        if (state->stack_cur == state->stack_end)
        {
            state->registers[14] = 0xDEAD;
            return;
        }

        *state->stack_cur++ = state->pc + 3;
        state->pc = abs.addr - 3;
    }
};

struct ret
{
    constexpr void operator()(svm::vm_state* state)
    {
        if (state->stack_cur == state->stack_begin)
        {
            state->registers[14] = 0xDEAD;
            return;
        }

        state->pc = *--state->stack_cur - 1;
    }
};

struct read_byte
{
    void operator()(svm::vm_state* state, tvm::reg_ind_t<4> to, tvm::reg_ind_t<4> reg)
    {
        auto ptr = reinterpret_cast<uint8_t*>(state->registers[reg.index]);
        state->registers[to.index] = *ptr;
    }
};

struct read_word
{
    void operator()(svm::vm_state* state, tvm::reg_ind_t<4> to, tvm::reg_ind_t<4> reg)
    {
        auto ptr = reinterpret_cast<uint16_t*>(state->registers[reg.index]);
        state->registers[to.index] = *ptr;
    }
};

namespace tvm
{
    template<>
    struct instr_name<movi>
    {
        static constexpr auto value() { return "movi"; }
    };

    template<>
    struct instr_name<add>
    {
        static constexpr auto value() { return "add"; }
    };

    template<>
    struct instr_name<movr>
    {
        static constexpr auto value() { return "movr"; }
    };

    template <>
    struct instr_name<jump>
    {
        static constexpr auto value() { return "jump"; }
    };

    template <>
    struct instr_name<jumpi>
    {
        static constexpr auto value() { return "jumpi"; }
    };

    template <>
    struct instr_name<call>
    {
        static constexpr auto value() { return "call"; }
    };

    template <>
    struct instr_name<ret>
    {
        static constexpr auto value() { return "ret"; }
    };
    template<>
    struct instr_name<branch_if_eq>
    {
        static constexpr auto value() { return "beq"; }
    };

    template<>
    struct instr_name<exit_ins>
    {
        static constexpr auto value() { return "exit"; }
    };

    template<>
    struct instr_name<push>
    {
        static constexpr auto value() { return "push"; }
    };

    template<>
    struct instr_name<pop>
    {
        static constexpr auto value() { return "pop"; }
    };

    template<>
    struct instr_name<read_byte>
    {
        static constexpr auto value() { return "rb"; }
    };

    template<>
    struct instr_name<read_word>
    {
        static constexpr auto value() { return "rw"; }
    };
}