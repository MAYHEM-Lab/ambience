//
// Created by fatih on 4/19/18.
//

#pragma once

#include <stdint.h>

struct proc_flags
{
};

struct vm_state
{
    uint16_t registers[16];
    uint16_t pc;
};
