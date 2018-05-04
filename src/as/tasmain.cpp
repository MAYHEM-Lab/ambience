//
// Created by fatih on 4/20/18.
//

#include <fstream>
#include <tvm/as/scanner.hpp>
#include <iostream>
#include <tvm/instructions.hpp>
#include <tvm/as/grammar.hpp>
#include <tvm/as/parser.hpp>
#include <tvm/as/ast_printer.hpp>
#include <tvm/as/isa_description.hpp>
#include <tvm/as/codegen.hpp>

using ISA = tvm::list <
        tvm::ins<0x01, add>,
        tvm::ins<0x02, movi>,
        tvm::ins<0x03, jump>,
        tvm::ins<0x04, branch_if_eq>,
        tvm::ins<0x05, exit_ins>,
        tvm::ins<0x06, movr>
    >;
using isa_t = tvm::isa_map<ISA>;
static constexpr isa_t isa{};

auto isa_descr = tvm::describe(ISA{});

void print_instr(tvm::instr_data& inst)
{
    std::cout << inst.mnemonic() << std::hex << " (0x" << (int)inst.get_opcode() << ")" << std::dec << '\n';
    for (auto& opc : inst.get_operands())
    {
        std::cout << "\tOp: " << (int)opc.type << " " << (int)opc.bits << " bits" << '\n';
    }
    for (auto off : inst.get_offsets())
    {
        std::cout << "\t" << off;
    }
    std::cout << '\n';
}

int main()
{
    std::ifstream prog("../bc/loop_label.tcs");
    tvm::as::scanner s{prog};

    std::ifstream dup("../bc/loop_label.tcs");
    tvm::as::parser p{ dup, s.begin() };

    auto parsed = p.parse_program();
    //std::cout << parsed.size() << '\n';

    std::ofstream res("a.out", std::ios::binary);
    tvm::as::codegen dg{parsed, isa_descr};
    dg.generate(res);
    return 0;

    for (auto& elem : parsed)
    {
        mpark::visit(tvm::as::print_ast{std::cout, 0}, elem);
        std::cout << '\n';
    }
}