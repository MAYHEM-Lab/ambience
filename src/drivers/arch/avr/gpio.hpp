//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <stdint.h>

namespace tos
{
    class avr_gpio
    {
    public:
        using pin_id_t = uint8_t;
        enum class pin_mode_t
        {
            out,
            in
        };
        using digital_io_t = bool;

        void set_pin_mode(pin_id_t, pin_mode_t);
        void write(pin_id_t, digital_io_t);
    };
}