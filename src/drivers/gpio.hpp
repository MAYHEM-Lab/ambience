//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

namespace tos
{
    struct pin_id
    {
        uint8_t pin;
    };

    constexpr pin_id operator""_pin(unsigned long long pin)
    {
        return {(uint8_t)pin};
    }
}
