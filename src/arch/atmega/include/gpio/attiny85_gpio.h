#ifndef attiny85_gpio_h
#define attiny85_gpio_h

#include <stdint.h>
#include "../gpio.h"

/*
Source            Atmel.ATtiny_DFP.1.3.229.atpack
Family            tinyAVR
Architecture      AVR8
Device name       ATtiny85
*/

namespace GPIO {


    auto const PortB = GPIO::Port {
        0x38,
        0x36,
        0x37,
    };


        auto const PB0 = GPIO::Pin {
            PortB,
            GPIO::Pin0,
        };


        auto const PB1 = GPIO::Pin {
            PortB,
            GPIO::Pin1,
        };


        auto const PB2 = GPIO::Pin {
            PortB,
            GPIO::Pin2,
        };


        auto const PB3 = GPIO::Pin {
            PortB,
            GPIO::Pin3,
        };


        auto const PB4 = GPIO::Pin {
            PortB,
            GPIO::Pin4,
        };


        auto const PB5 = GPIO::Pin {
            PortB,
            GPIO::Pin5,
        };


        auto const PB6 = GPIO::Pin {
            PortB,
            GPIO::Pin6,
        };


        auto const PB7 = GPIO::Pin {
            PortB,
            GPIO::Pin7,
        };



}

#endif
