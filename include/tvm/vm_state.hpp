//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>
#include <array>

struct vm_state
{
    std::array<uint16_t, 16> registers;
};
