//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/devices.hpp>
#include <stdint.h>

namespace tos
{
    namespace pin_mode
    {
        struct input_t{};
        struct output_t{};

        struct in_pullup_t{};
        struct in_pulldown_t{};

        struct alternate_output_t {};

        static constexpr input_t in{};
        static constexpr output_t out{};

        static constexpr in_pullup_t in_pullup{};
        static constexpr in_pulldown_t in_pulldown{};

        static constexpr alternate_output_t alt_out{};

        static constexpr struct fast_t {} fast;
    } // namespace pin_mode

    namespace digital
    {
        using high_t = std::true_type;
        using low_t = std::false_type;

        static constexpr high_t high{};
        static constexpr low_t low{};
    } // namespace digital

    namespace pin_change
    {
        struct falling_t {};
        struct rising_t {};

        static constexpr falling_t falling{};
        static constexpr rising_t rising{};
    }

    /**
     * This type implements a scope based guard that resets
     * a pin in it's constructor and sets it in it's
     * destructor
     */
    template <class GpioT>
    class pull_low_guard
    {
    public:
        using pin_type = typename GpioT::pin_type;
        pull_low_guard(GpioT& g, pin_type pin)
            : m_g{&g}, m_pin{pin}
        {
            m_g->write(m_pin, tos::digital::low);
        }

        ~pull_low_guard()
        {
            m_g->write(m_pin, tos::digital::high);
        }
    private:
        GpioT* m_g;
        pin_type m_pin;
    };

    namespace devs
    {
        using gpio_t = dev<struct _gpio_t, 0>;
        static constexpr gpio_t gpio{};
    } // namespace devs
} // namespace tos
