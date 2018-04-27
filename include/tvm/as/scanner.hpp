
#pragma once

#include <iosfwd>
#include <type_traits>
#include <iterator>
#include "token.hpp"

namespace tvm::as
{
    class token_iterator_end
            : public std::iterator<token, std::iterator_traits<std::forward_iterator_tag>>
    {
    };
    class token_iterator
            : public std::iterator<token, std::iterator_traits<std::forward_iterator_tag>>
    {
        class scanner *m_scanner;
        token m_last_read;

    public:
        explicit token_iterator(scanner* sc);
        token_iterator& operator++();
        token operator*();
        bool operator==(token_iterator_end);
        bool operator!=(token_iterator_end);
    };

    class scanner
    {
        std::istream* m_src;

        std::istream& get_stream() { return *m_src; }

        token_types try_tokenize(char next) const;
        token_types try_tokenize(token_types current, char next, std::integral_constant<int, 2>) const { return token_types::invalid; }
        token_types try_tokenize(token_types current, char next, std::integral_constant<int, 3>) const { return token_types::invalid; }

        bool is_whitespace(char) const;

        void error(const char*, int pos);
        token tokenize_string();
        token tokenize_number(char);
        token tokenize_name(char);
        friend class token_iterator;

        token next_token();
    public:

        explicit scanner(std::istream& src) : m_src(&src) {}

        token_iterator begin() { return token_iterator{this}; }
        token_iterator_end end() { return {}; }
    };
}
