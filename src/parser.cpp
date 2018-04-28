//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#include <tvm/as/parser.hpp>

namespace tvm::as
{
    program parser::parse_program()
    {
        program p;
        while (m_pos != token_iterator_end{})
        {
            p.push_back(parse_entity());
        }
        return p;
    }

    entity parser::parse_entity()
    {
        auto tok = *m_pos;
        switch (tok.type)
        {
        case token_types::line_comment:
            ++m_pos;
            return line_comment{tok};
        case token_types::block_comment:
            ++m_pos;
            return blk_comment{tok};
        case token_types::name:
            return parse_instruction();
        default:
            throw parse_error("bad");
        }
    }

    std::string parser::read_tok(token tok)
    {
        m_is.seekg(tok.pos, std::ios::beg);
        std::string res(tok.length, '\0');
        m_is.read(&res[0], tok.length);
        return res;
    }

    name parser::parse_name()
    {
        auto tok = *m_pos;
        if (tok.type != token_types::name)
        {
            throw parse_error("bad");
        }
        ++m_pos;

        return { tok, read_tok(tok) };
    }

    literal parser::parse_literal()
    {
        auto tok = *m_pos;
        switch (tok.type)
        {
        case token_types::integer_literal:
            ++m_pos;
            return float_lit{ tok, std::stoll(read_tok(tok)) };
        case token_types::float_literal:
            ++m_pos;
            return float_lit{ tok, std::stof(read_tok(tok)) };
        default:
            throw parse_error("bad");
        }
    }

    label parser::parse_label()
    {
        auto tok = *m_pos;
        if (tok.type != token_types::label_name)
        {
            throw parse_error("bad");
        }
        ++m_pos;

        return { tok, read_tok(tok) };
    }

    register_ parser::parse_register()
    {
        auto tok = *m_pos;
        if (tok.type != token_types::reg_name)
        {
            throw parse_error("bad");
        }
        ++m_pos;

        return { tok, read_tok(tok) };
    }

    operand parser::parse_operand()
    {
        try {
            return parse_literal();
        }
        catch (...) {}
        try {
            return parse_label();
        }
        catch (...) {}
        return parse_register();
    }

    operands parser::parse_operands()
    {
        operands res;
        while (true)
        {
            try
            {
                res.push_back(parse_operand());
            }
            catch (parse_error& err)
            {
                break;
            }
        }
        return res;
    }

    instruction parser::parse_instruction()
    {
        auto name = parse_name();
        auto ops = parse_operands();
        return { name, ops };
    }
}