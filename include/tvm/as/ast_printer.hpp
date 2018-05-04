//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#pragma once

#include <ostream>
#include <variant.hpp>
#include <tvm/as/grammar.hpp>

namespace tvm::as {
    inline void indent(std::ostream& os, uint8_t x, const std::string& ind = "\t")
    {
        for (int i = 0; i<x; ++i) {
            os << ind;
        }
    }

    class ast_printer
    {
    public:
        ast_printer(std::ostream& os, uint8_t depth)
                :m_os{os}, m_depth{depth}
        { }

    protected:
        void indent()
        {
            ::tvm::as::indent(m_os, m_depth);
        }

        std::ostream& m_os;
        uint8_t m_depth;
    };

    struct print_literal : ast_printer
    {
        using ast_printer::ast_printer;

        void operator()(const tvm::as::int_lit& l)
        {
            indent();
            m_os << std::hex << l.val << std::dec;
        }

        void operator()(const tvm::as::float_lit& l)
        {
            indent();
            m_os << l.val;
        }
    };

    struct print_operand : ast_printer
    {
        using ast_printer::ast_printer;

        void operator()(const tvm::as::register_& reg)
        {
            indent();
            m_os << "Reg(" << reg.name << ")";
        }

        void operator()(const tvm::as::label& lbl)
        {
            indent();
            m_os << "Label(" << lbl.name << ")";
        }

        void operator()(const tvm::as::literal& lit)
        {
            indent();
            m_os << "Literal(";
            mpark::visit(print_literal{m_os, 0}, lit);
            m_os << ")";
        }
    };

    struct print_ast : ast_printer
    {
        using ast_printer::ast_printer;

        void operator()(const tvm::as::blk_comment& comm)
        {
            indent();
            m_os << "Block Comment";
        }

        void operator()(const tvm::as::line_comment& comm)
        {
            indent();
            m_os << "Line Comment";
        }

        void operator()(const tvm::as::instruction& instr)
        {
            indent();
            m_os << "Instruction\n";
            ::tvm::as::indent(m_os, m_depth+1);
            m_os << std::get<tvm::as::name>(instr).name;
            for (auto& op : std::get<tvm::as::operands>(instr)) {
                m_os << '\n';
                mpark::visit(print_operand{m_os, m_depth+1}, op);
            }
        }

        template<class T>
        void operator()(const T& other)
        {
            indent();
            m_os << "other";
        }
    };
}

