//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#pragma once

#include "instructions.hpp"

namespace svm
{
    using ISA = tvm::list <
        tvm::ins<0x01, add>,
        tvm::ins<0x02, movi>,
        tvm::ins<0x03, jump>,
        tvm::ins<0x04, branch_if_eq>,
        tvm::ins<0x05, exit_ins>,
        tvm::ins<0x06, movr>
    >;
}
