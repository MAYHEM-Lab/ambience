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
        case token_types::new_line:
            ++m_pos;
            return parse_entity();
        case token_types::label_name:
            return parse_label();
        default:
            throw parse_error("Unexpected token in file");
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
            return int_lit{ tok, std::stoull(read_tok(tok), 0, 0) };
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

        auto name = read_tok(tok);
        name.pop_back();
        return { tok, name };
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
            return parse_name();
        }
        catch (...) {}
        try {
            return parse_register();
        }
        catch (...) {}
        throw missing_operand_error("can't parse operand");
    }

    operands parser::parse_operands()
    {
        operands res;
        while (true)
        {
            auto next = *m_pos;
            if (next.type == token_types::new_line ||
                next.type == token_types::line_comment)
            {
                ++m_pos;
                break;
            }
            res.push_back(parse_operand());
            next = *m_pos;
            if (next.type == token_types::new_line ||
                next.type == token_types::line_comment)
            {
                break;
            }
            if (next.type != token_types::comma)
            {
                throw missing_comma_error("Expected comma between operands");
            }
            ++m_pos;
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