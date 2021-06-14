//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#pragma once

#include <istream>
#include <tvm/as/scanner.hpp>
#include <tvm/as/grammar.hpp>

namespace tvm::as
{
    class parse_error : public std::runtime_error
    {
    public:
        parse_error(const std::string& err) : runtime_error(err) {}
    };

    class missing_comma_error : public parse_error
    {
    public:
        using parse_error::parse_error;
    };

    class missing_operand_error : public parse_error
    {
    public:
        using parse_error::parse_error;
    };

    class parser
    {
    public:
        parser(std::istream& is, token_iterator it)
                : m_is{is}, m_pos{it} {}

        program parse_program();

    private:

        std::string read_tok(token t);

        entity parse_entity();
        name parse_name();
        label parse_label();
        register_ parse_register();
        instruction parse_instruction();
        operands parse_operands();
        operand parse_operand();
        literal parse_literal();

        std::istream& m_is;
        token_iterator m_pos;
    };
}
