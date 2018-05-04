//
// Created by fatih on 4/20/18.
//

#include <fstream>
#include <tvm/as/scanner.hpp>
#include <iostream>
#include "instructions.hpp"
#include <tvm/as/grammar.hpp>
#include <tvm/as/parser.hpp>
#include <tvm/as/ast_printer.hpp>
#include <tvm/as/isa_description.hpp>
#include <tvm/as/codegen.hpp>
#include "vm_def.hpp"

using isa_t = tvm::isa_map<svm::ISA>;
static constexpr isa_t isa{};

auto isa_descr = tvm::describe(svm::ISA{});

int main(int argc, char** argv)
{
    std::ifstream prog(argv[1]);
    tvm::as::scanner s{prog};

    std::ifstream dup(argv[1]);
    tvm::as::parser p{ dup, s.begin() };

    auto parsed = p.parse_program();
    //std::cout << parsed.size() << '\n';

    std::ofstream res(argv[2], std::ios::binary);
    tvm::as::codegen dg{parsed, isa_descr};
    dg.generate(res);
    return 0;

    for (auto& elem : parsed)
    {
        mpark::visit(tvm::as::print_ast{std::cout, 0}, elem);
        std::cout << '\n';
    }
}