//
// Created by Mehmet Fatih BAKIR on 27/04/2018.
//

#pragma once

#include <tvm/as/grammar.hpp>
#include <tvm/as/isa_description.hpp>
#include <ostream>

namespace tvm::as
{
    class codegen_error : public std::runtime_error
    {
        using runtime_error::runtime_error;
    };

    class codegen
    {
    public:
        codegen(program, isa_description);

        void generate(std::ostream&);

    private:
        program m_prog;
        isa_description m_descr;
    };
}
