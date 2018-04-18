//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <stdint.h>

namespace tos
{
namespace avr
{
    enum class ports
    {
        B, C, D
    };

    class gpio
    {
    public:
        using pin_id_t = uint8_t;
        enum class pin_mode_t
        {
            out,
            in
        };

        using digital_io_t = bool;

        void set_pin_mode(ports port, pin_id_t, pin_mode_t);
        void write(ports port, pin_id_t, digital_io_t);
    };
}
}

///// Implementation

#include <avr/io.h>

namespace tos
{
    namespace avr
    {
        namespace impl
        {
            static constexpr auto& get_dir(ports port)
            {
                switch (port)
                {
                    case ports::B: return DDRB;
                    case ports::C: return DDRC;
                    case ports::D: return DDRD;
                }
                return DDRB;
            }

            static constexpr auto& get_port(ports port)
            {
                switch (port)
                {
                    case ports::B: return PORTB;
                    case ports::C: return PORTC;
                    case ports::D: return PORTD;
                }
                return PORTB;
            }
        }

        inline void gpio::set_pin_mode(ports port, gpio::pin_id_t pin, gpio::pin_mode_t mode)
        {
            if (mode == pin_mode_t::out)
            {
                impl::get_dir(port) |= (1 << pin);
            }
            else // if (mode == pin_mode_t::in)
            {
                impl::get_dir(port) &= ~(1 << pin);
            }
        }

        inline void gpio::write(ports port, gpio::pin_id_t pin, gpio::digital_io_t what)
        {
            if (what)
            {
                impl::get_port(port) |= (1 << pin);
            }
            else {
                impl::get_port(port) &= ~(1 << pin);
            }
        }
    }
}