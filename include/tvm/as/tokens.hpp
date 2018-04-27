//
// Created by fatih on 4/27/18.
//

#pragma once

namespace tvm::as
{
    enum class token_types
    {
        comma, // ,
        colon, // :
        question, // ?

        dot, // .

        semicolon, // ;

        plus, // +
        minus, // -
        slash, // /
        star, // *

        percent, // %

        name, // add
        reg_name, // %r0
        label_name, // main:

        string_literal, // "hi"
        integer_literal, // 42
        float_literal, // 3.14

        eof,

        line_comment, // //like this
        block_comment, // /* like this */

        double_quote, // "
        single_quote, // '

        invalid
    };
}