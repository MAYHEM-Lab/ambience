//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#include <tvm/as/codegen.hpp>
#include <iostream>

using namespace tvm::as;
struct codegen_visitor
{
    codegen_visitor(const tvm::isa_description& d, std::ostream& os) : m_descr{d}, m_os{os} {}
protected:
    const tvm::isa_description& m_descr;
    std::ostream& m_os;
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
        switch (args[i].type)
        {
        case tvm::operand_type::reg:
            if (!mpark::get_if<register_>(&ops[i]))
            {
                throw codegen_error("operand types do not match!");
                return false;
            }
            break;
        case tvm::operand_type::literal:
            if (!mpark::get_if<literal>(&ops[i]))
            {
                throw codegen_error("operand types do not match!");
                return false;
            }
            break;
        case tvm::operand_type::address:
            if (!mpark::get_if<literal>(&ops[i]) && !mpark::get_if<name>(&ops[i]))
            {
                throw codegen_error("operand types do not match!");
            }
            break;
        }

        //TODO: if types match, make sure sizes match too!
    }
    return true;
}

std::unordered_map<std::string, uint16_t> symbols;
struct missing_info
{
    tvm::as::instruction at;
    std::string label;
};
std::unordered_map<uint16_t, missing_info> missing;

enum class operand_type
{
    label,
    literal,
    reg
};

struct operand_result
{
    operand_type type;
    uint32_t value;
};

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

    operand_result operator()(const literal& l)
    {
        return { operand_type::literal, mpark::visit(literal_visitor{}, l) };
    }

    operand_result operator()(const register_& r)
    {
        auto num = r.name.substr(2);
        return { operand_type::reg, std::stoul(num) };
    }

    operand_result operator()(const name& n)
    {
        //std::cout << "name: " << n.name << '\n';
        auto res = 0;
        auto it = symbols.find(n.name);
        if (it != symbols.end())
        {
            res = it->second;
        }
        return { operand_type::label, res };
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
        auto bits = i_descr->get_size_bits();
        auto end = 0;
        result = accum(result, i_descr->get_opcode(),
                bits - m_descr.get_opcode_size(), m_descr.get_opcode_size());
        for (int i = i_descr->operand_count() - 1; i >= 0; --i)
        {
            operand_result r = mpark::visit(operand_visitor{}, ops[i]);
            auto val = r.value;
            if (r.type == operand_type::label && val == 0)
            {
                // we have a missing label
                missing[m_os.tellp()] = { ins, mpark::get<tvm::as::name>(ops[i]).name };
                m_os.seekp(i_descr->get_size(), std::ios::cur);
                return;
            }
            result = accum(result, val, offsets[i], operands[i].bits);
            end = offsets[i] + operands[i].bits;
        }
        result <<= 32 - (end + m_descr.get_opcode_size());
        auto ptr = reinterpret_cast<const char*>(&result);
        for (int i = 0; i < i_descr->get_size(); ++i)
        {
            m_os.write(ptr + 3 - i, 1);
        }
    }

    void operator()(const tvm::as::label& lbl)
    {
        symbols[lbl.name] = uint16_t(m_os.tellp());
    }

    void operator()(const tvm::as::blk_comment&) {}
    void operator()(const tvm::as::line_comment&) {}
};

namespace tvm::as
{
    codegen::codegen(program p, isa_description d) :
        m_prog(std::move(p)), m_descr(std::move(d)) { }

    void codegen::generate(std::ostream& out)
    {
        auto vis = entity_visitor{m_descr, out};
        for (auto& e : m_prog)
        {
            mpark::visit(vis, e);
        }
        uint32_t zeros = 0;
        out.write(reinterpret_cast<const char*>(&zeros), 4);

        auto miss = std::move(missing);
        for (auto& [off, ins] : miss)
        {
            out.seekp(off, std::ios::beg);
            vis(ins.at);
        }

        for (auto& [off, ins] : missing)
        {
            std::cerr << "undefined symbol: " << ins.label << '\n';
        }
        if (!missing.empty())
        {
            throw codegen_error("undefined symbols exist");
        }
    }
}