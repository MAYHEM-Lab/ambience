//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#include <tvm/as/codegen.hpp>
#include <iostream>

using namespace tvm::as;
struct codegen_visitor
{
    codegen_visitor(const tvm::isa_description& d) : m_descr{d} {}
protected:
    const tvm::isa_description& m_descr;
};

struct entity_visitor : codegen_visitor
{
    using codegen_visitor::codegen_visitor;
    void operator()(const instruction& i)
    {
        auto& n = std::get<name>(i).name;
        const auto data = m_descr.get(n);
        if (!data)
        {
            throw codegen_error("undefined instruction " + n);
        }
        std::cout << n << " " << "ok!\n";
        const auto op_count = data->operand_count();
        auto& ops = std::get<operands>(i);
        if (ops.size() != op_count)
        {
            throw codegen_error("incorrect number of arguments " + n);
        }
    }

    template <class T>
    void operator()(const T&){}
};

namespace tvm::as
{
    codegen::codegen(program p, isa_description d) :
        m_prog(std::move(p)), m_descr(std::move(d)) { }

    void codegen::generate()
    {
        for (auto& e : m_prog)
        {
            mpark::visit(entity_visitor{m_descr}, e);
        }
    }
}