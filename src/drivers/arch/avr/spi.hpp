//
// Created by fatih on 4/15/18.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>
#include <drivers/common/spi.hpp>
#include <util/include/tos/span.hpp>
#include "gpio.hpp"

namespace tos {
    namespace avr {
        class spi0
        {
        public:
            using gpio_type = avr::gpio;

            static void init_master();

            static void init_slave();

            static void enable();

            static void disable();

            static uint8_t exchange(uint8_t byte);
            static void exchange_many(tos::span<uint8_t> buffer);

            static void select_slave(pin_t pin);

            static void deselect_slave(pin_t pin);

            spi0() = default;

            spi0*operator->() { return this; }
            spi0&operator*() { return *this; }
        };
    }

    inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::master_t /*type*/)
    {
        avr::spi0::init_master();
        return {};
    }

    inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::slave_t /*type*/)
    {
        avr::spi0::init_slave();
        return {};
    }
}