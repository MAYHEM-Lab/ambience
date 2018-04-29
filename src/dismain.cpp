#include <iostream>

#include <tvm/meta.hpp>
#include <tvm/vm_state.hpp>
#include <tvm/tvm_types.hpp>
#include <tvm/traits.hpp>
#include <tvm/instr_traits.hpp>
#include <tvm/exec/decoding.hpp>
#include <tvm/dis/disassemble.hpp>
#include <tvm/instructions.hpp>

#include <unordered_map>
#include <vector>
#include <fstream>

using ISA = tvm::list <tvm::ins<0x01, add>, tvm::ins<0x02, movi>, tvm::ins<0x03, exit_ins>>;

std::ostream& operator<<(std::ostream& os, const tvm::reg_ind_t<>& a)
{
    return os << "%r" << (int)a.index;
}

std::ostream& operator<<(std::ostream& os, const tvm::operand_t<16>& a)
{
    return os << a.operand;
}

template<class TupType, size_t... I>
void print(std::ostream& os, const TupType& _tup, std::index_sequence<I...>)
{
    (..., (os << (I == 0? "" : ", ") << std::get<I>(_tup)));
}

template<class... T>
void print (std::ostream& os, const std::tuple<T...>& _tup)
{
    print(os, _tup, std::make_index_sequence<sizeof...(T)>());
}

template <uint8_t N>
constexpr tvm::dis::printer get_printer(tvm::opcode_t<N>  c)
{
    switch (c.opcode)
    {
        case 0x01: return tvm::dis::print_args<add>();
        case 0x02: return tvm::dis::print_args<movi>();
        case 0x03: return tvm::dis::print_args<exit_ins>();
        default:
            return nullptr;
    }
}

uint8_t disas_one(std::ostream& os, uint32_t instr)
{
    return get_printer(tvm::get_opcode<7>(instr))(os, instr);
}

template <class Instrs>
void disas(const Instrs& prog)
{
    auto* pos = prog;
    tvm::vm_state state{};

    auto load = [&pos]{
        auto p = pos;
        uint32_t res = 0;
        res |= *p++; res <<= 8;
        res |= *p++; res <<= 8;
        res |= *p++; res <<= 8;
        res |= *p++;
        return res;
    };

    pos += disas_one(std::cout, load());
    std::cout << '\n';
    pos += disas_one(std::cout, load());
    std::cout << '\n';
    pos += disas_one(std::cout, load());
    std::cout << '\n';
}

int main() {
    std::ifstream in("a.out", std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    uint8_t prog[12] = {
            0x4, 0x0, 0x0, 0x40, // mov %r0, 2
            0x4, 0x20, 0x0, 0x80, // mov %r1, 4
            0x2, 0x2, 0x0, 0x0 // add %r0, %r1
    };

    disas(contents.data());

    return 0;
}