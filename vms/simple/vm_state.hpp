//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include <stdint.h>

namespace svm
{
    struct vm_state
    {
        uint16_t registers[15];
        uint16_t pc;

        uint16_t* stack_begin;
        uint16_t* stack_cur;
        uint16_t* stack_end;

        bool alive() const {
            return registers[14] != 0xDEAD;
        }
    };
}
