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

bool type_check(const tvm::instr_data& id, const instruction& ins)
{
    const auto op_count = id.operand_count();
    auto& ops = std::get<operands>(ins);
    if (ops.size() != op_count)
    {
        throw codegen_error("incorrect number of arguments for " + std::string(id.mnemonic()));
    }
    const auto& args = id.get_operands();
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i].type == tvm::operand_type::reg && !mpark::get_if<register_>(&ops[i]))
        {
            throw codegen_error("operand types do not match!");
        }
        else if (args[i].type == tvm::operand_type::literal && !mpark::get_if<literal>(&ops[i]))
        {
            throw codegen_error("operand types do not match!");
        }
        //TODO: if types match, make sure sizes match too!
    }
    return true;
}

struct operand_visitor
{
    struct literal_visitor
    {
        uint32_t operator()(const int_lit& il)
        {
            return uint32_t (il.val);
        }

        uint32_t operator()(const float_lit& fl)
        {
            return uint32_t (fl.val);
        }
    };

    uint32_t operator()(const literal& l)
    {
        auto res = mpark::visit(literal_visitor{}, l);
        return res;
    }

    uint32_t operator()(const register_& r)
    {
        auto num = r.name.substr(2);
        return std::stoul(num);
    }

    uint32_t operator()(const label&)
    {
        return 0;
    }
};

uint32_t accum(uint32_t prev, uint32_t val, uint32_t offset, uint32_t sz)
{
    auto mask = (1U << sz) - 1U;
    val &= mask;
    val <<= offset;
    prev |= val;
    return prev;
}

struct entity_visitor : codegen_visitor
{
    using codegen_visitor::codegen_visitor;
    void operator()(const instruction& ins)
    {
        auto& n = std::get<name>(ins).name;
        const auto i_descr = m_descr.get(n);

        if (!i_descr)
        {
            throw codegen_error("undefined instruction " + n);
        }
        if (!type_check(*i_descr, ins)) return;

        // instruction exists and operands match, generate code for it

        auto& ops = std::get<operands>(ins);

        uint32_t result = 0;
        auto operands = i_descr->get_operands();
        auto offsets = i_descr->get_offsets();
        auto end = 0;
        for (int i = i_descr->operand_count() - 1; i >= 0; --i)
        {
            uint32_t val = mpark::visit(operand_visitor{}, ops[i]);
            std::cout << "put " << int(operands[i].bits) << " bits of " << val << " at " << offsets[i] << '\n';
            result = accum(result, val, offsets[i], operands[i].bits);
            end = offsets[i] + operands[i].bits;
        }
        std::cout << "put " << 7 << " bits of " << int(i_descr->get_opcode()) << " at " << end << '\n';
        result = accum(result, i_descr->get_opcode(), end, 7);
        result <<= 32 - (end + 7);
        std::cout << std::hex << result << std::dec << '\n';
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