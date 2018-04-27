//
// Created by fatih on 4/27/18.
//

#pragma once

#include "tokens.hpp"
#include <cstdint>

namespace tvm::as
{
    struct token
    {
        token_types type;
        int32_t pos;
        uint16_t length;
    };
}
