//
// Created by fatih on 4/27/18.
//

#include <tvm/as/scanner.hpp>
#include <istream>
#include <array>
#include <algorithm>

namespace tvm::as
{
    token_types scanner::try_tokenize(char next) const
    {
        switch (next)
        {
            case '+': return token_types::plus;
            case '-': return token_types::minus;
            case '/': return token_types::slash;
            case '*': return token_types::star;
            case '.': return token_types::dot;
            case ',': return token_types::comma;
            case ':': return token_types::colon;
            case ';': return token_types::semicolon;
            case '\'': return token_types::single_quote;
            case '\"': return token_types::double_quote;
            case '?' : return token_types::question;
            case '%' : return token_types::percent;
            case std::istream::traits_type::eof(): return token_types::eof;
            default: return token_types::invalid;
        }
    }

    bool scanner::is_whitespace(char next) const
    {
        switch(next){
            case '\n':
            case '\t':
            case ' ':
                return true;
            default:
                return false;
        }
    }

    void scanner::error(const char* err, int pos)
    {
        throw std::runtime_error("err at " + std::to_string(pos) + " : " + err);
    }

    token scanner::tokenize_string()
    {
        auto begin = get_stream().tellg();
        auto next = get_stream().get();
        auto len = 0;
        while (try_tokenize(next) != token_types::double_quote)
        {
            next = get_stream().get();
            if (next == '\n')
            {
                error("Newline in string", get_stream().tellg());
            }
            if (try_tokenize(next) == token_types::eof)
            {
                error("Unterminated string", get_stream().tellg());
            }
            ++len;
        }
        return { token_types::string_literal, (int32_t)begin, (uint16_t)len };
    }

    token scanner::tokenize_number(char next)
    {
        auto can_be_in_number = [this](char elem){
            if (!isalnum(elem) && try_tokenize(elem) != token_types::dot){
                return false;
            }
            if (isalpha(elem))
            {
                return false;
            }

            return true;
        };

        auto begin = size_t(get_stream().tellg()) - 1U;
        auto len = 1;

        bool has_dot = false;
        bool one_dot = false;
        while (can_be_in_number(next = get_stream().peek()))
        {
            get_stream().get();
            len++;
            if (try_tokenize(next) == token_types::dot)
            {
                has_dot = true;
                if (!one_dot)
                {
                    one_dot = true;
                }
                else
                {
                    error("bad numeric literal", get_stream().tellg());
                }
            }
            else
            {
                one_dot = false;
            }
        }

        return { has_dot ? token_types::float_literal : token_types::integer_literal, (int32_t)begin, (uint16_t)len };
    }

    token scanner::tokenize_name(char next)
    {
        auto can_be_in_name = [this](char elem) {
            return isalnum(elem) || elem == '_';
        };

        auto begin = size_t(get_stream().tellg()) - 1;
        auto len = 1;

        while (can_be_in_name(next = get_stream().get()))
        {
            len++;
        }

        return { token_types::name, (int32_t)begin, uint16_t(len) };
    }

    token scanner::next_token()
    {
        auto began = size_t(get_stream().tellg());
        auto len = 0;

        auto next = get_stream().get();

        while (is_whitespace(next))
        {
            began = get_stream().tellg();
            next = get_stream().get();
        }

        if ((bool)isalpha(next))
        {
            // maybe a name or keyword
            return tokenize_name(next);
        }

        if (isalnum(next) && !isalpha(next))
        {
            return tokenize_number(next);
        }

        token_types tok_type = try_tokenize(next);

        if (tok_type == token_types::percent)
        {
            auto reg_name = tokenize_name(next);
            reg_name.type = token_types::reg_name;
            return reg_name;
        }

        if (tok_type == token_types::line_comment)
        {
            while ((next = get_stream().get()) != '\n')
            {
                len++;
            }
        }

        if (tok_type == token_types::block_comment)
        {
            auto prev = next;
            do {
                prev = next;
                next = get_stream().get();
                len++;
            } while (!(prev == '*' && next == '/'));
        }

        if (tok_type == token_types::double_quote)
        {
            return tokenize_string();
        }

        return { tok_type, (int32_t)began, (uint16_t)len };
    }

    token_iterator &token_iterator::operator++() {
        m_last_read = m_scanner->next_token();
        return *this;
    }

    token token_iterator::operator*() {
        return m_last_read;
    }

    bool token_iterator::operator==(token_iterator_end) {
        return m_last_read.type == token_types::eof;
    }

    bool token_iterator::operator!=(token_iterator_end) {
        return !(*this == token_iterator_end{});
    }

    token_iterator::token_iterator(scanner *sc) : m_scanner(sc) {
        m_last_read = m_scanner->next_token();
    }
}