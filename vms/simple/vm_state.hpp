//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include <stdint.h>

namespace svm
{
    struct vm_state
    {
        uint16_t registers[16];
        uint16_t pc;
    };
}
