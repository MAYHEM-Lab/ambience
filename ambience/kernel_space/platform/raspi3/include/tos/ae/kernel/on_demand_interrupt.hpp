#pragma once

#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/arch.hpp>

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(T&& t) {
        tos::cur_arch::exception::set_svc_handler(
            tos::cur_arch::exception::svc_handler_t(t));
        tos::cur_arch::svc1();
    }
};