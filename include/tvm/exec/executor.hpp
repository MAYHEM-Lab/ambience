//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include <tvm/exec/isa.hpp>
#ifdef TOS
#include <tos/utility.hpp>
using tos::move;
#else
using std::move;
#endif

namespace tvm
{
template <class VmT, class ISA, uint8_t N>
constexpr tvm::executor<VmT> get_executor(tvm::opcode_t<N> c)
{
    constexpr auto lookup = tvm::generate_decode_lookup<VmT, ISA>::value();
    return lookup.data[c.opcode];
}

template <uint8_t N, class ISA, class VmT>
constexpr uint8_t exec_one(VmT *state, uint32_t instr)
{
    return get_executor<VmT, ISA>(tvm::get_opcode<N>(instr))(state, instr);
}

template <class FetchT, class VmT, class IsaT>
struct vm_executor
{
    FetchT m_fetcher;
    VmT m_state;

    constexpr vm_executor(FetchT fetcher)
            : m_fetcher(move(fetcher)), m_state{}
    {}

    constexpr void exec_one();
    constexpr void exec();
};

template <class FetchT, class VmT, class IsaT>
constexpr void vm_executor<FetchT, VmT, IsaT>::exec_one()
{
    m_state.pc += tvm::exec_one<7, IsaT>(&m_state, m_fetcher.fetch(m_state.pc));
}

template <class FetchT, class VmT, class IsaT>
constexpr void vm_executor<FetchT, VmT, IsaT>::exec()
{
    while (m_state.registers[14] != 0xDEAD)
    {
        exec_one();
    }
}
}