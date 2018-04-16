//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//


#include <drivers/arch/avr/gpio.hpp>

namespace tos
{
    using gpio = avr::gpio;
}

namespace tos
{
    template <class T> T* open();
}
