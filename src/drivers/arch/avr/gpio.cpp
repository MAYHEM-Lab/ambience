//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <gpio.hpp>
#include <avr/io.h>

namespace tos
{
    void avr_gpio::set_pin_mode(avr_gpio::pin_id_t pin, avr_gpio::pin_mode_t mode)
    {
        if (mode == pin_mode_t::out)
        {
            DDRB |= _BV(DDB5);
        }
        else
        {
            DDRB &= ~_BV(DDB5);
        }
    }

    void avr_gpio::write(avr_gpio::pin_id_t pin, avr_gpio::digital_io_t what)
    {
        if (what)
        {
            PORTB |= _BV(PORTB5);
        }
        else {
            PORTB &= ~_BV(PORTB5);
        }
    }
}