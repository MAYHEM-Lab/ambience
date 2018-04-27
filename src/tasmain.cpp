//
// Created by fatih on 4/20/18.
//

#include <fstream>
#include <tvm/as/scanner.hpp>
#include <iostream>
#include <tvm/instructions.hpp>

using ISA = list <ins<0x01, add>, ins<0x02, mov>>;

int main()
{
    std::ifstream prog("../bc/add.tcs");
    tvm::as::scanner s{prog};

    auto print_tok = [&](tvm::as::token tok)
    {
        auto tmp = prog.tellg();
        prog.seekg(tok.pos);
        std::string res;
        res.resize(tok.length);
        prog.read(&res[0], tok.length);
        prog.seekg(tmp);
        return res;
    };

    for (auto tok : s)
    {
        std::cout << (int)tok.type << ", " << tok.pos << ", " << tok.length << '\n';
        std::cout << print_tok(tok) << '\n';
        std::cout << " ----- \n";
    }
}