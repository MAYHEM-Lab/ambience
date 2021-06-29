#pragma once

#include <tos/arch.hpp>
#include <tos/arm/nvic.hpp>

class svc_on_demand_interrupt {
public:
    svc_on_demand_interrupt() {
        NVIC_EnableIRQ(SVCall_IRQn);
        NVIC_SetPriority(SVCall_IRQn, 0);
    }

    template<class T>
    void operator()(T&& t) {
        tos::arm::exception::set_svc_handler(tos::arm::exception::svc_handler_t(t));
        tos::arm::svc1();
    }
};
