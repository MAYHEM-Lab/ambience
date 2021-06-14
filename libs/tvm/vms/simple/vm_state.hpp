//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include <stdint.h>
#include <tvm/tvm_types.hpp>

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

        template<uint8_t N>
        constexpr uint16_t& operator[](tvm::reg_ind_t<N> ind) const {
            return registers[ind.index];
        }

        template<uint8_t N>
        constexpr uint16_t& operator[](tvm::reg_ind_t<N> ind) {
            return registers[ind.index];
        }
    };
}
