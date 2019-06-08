#pragma once

#include <common/gpio.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <cstdint>
#include <array>

namespace tos {
namespace stm32 {
struct port_def {
    uintptr_t which;
    rcc_periph_clken rcc;
};

struct pin_t {
    const port_def *port;
    uint16_t pin;
};

constexpr inline std::array<port_def, 8> ports = {
    port_def {
        GPIOA,
        RCC_GPIOA
    }, port_def {
        GPIOB,
        RCC_GPIOB
    }, port_def {
        GPIOC,
        RCC_GPIOC
    }, port_def {
        GPIOD,
        RCC_GPIOD
    }, port_def {
        GPIOE,
        RCC_GPIOE
    }
};

class gpio {
public:
    using pin_type = pin_t;

    /**
     * Sets the given pin to be an output
     */
    void set_pin_mode(const pin_type &pin, pin_mode::output_t) {
        rcc_periph_clock_enable(pin.port->rcc);
#if defined(STM32F1)
        gpio_set_mode(pin.port->which, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin.pin);
#else
        gpio_mode_setup(pin.port->which, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin.pin);
#endif
    }

    /**
     * Sets the given pin to be an output, and configures it
     * to use the fastest IO speed as possible
     */
    void set_pin_mode(const pin_type &pin, pin_mode::output_t, pin_mode::fast_t) {
        rcc_periph_clock_enable(pin.port->rcc);
#if defined(STM32F1)
        gpio_set_mode(pin.port->which, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin.pin);
#else
        gpio_mode_setup(pin.port->which, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin.pin);
#endif
    }

    void set_pin_mode(const pin_type &pin, pin_mode::input_t) {
        rcc_periph_clock_enable(pin.port->rcc);
#if defined(STM32F1)
        gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, pin.pin);
#else
        gpio_mode_setup(pin.port->which, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pin.pin);
#endif
    }

    void set_pin_mode(const pin_type &pin, pin_mode::in_pullup_t) {
        rcc_periph_clock_enable(pin.port->rcc);
#if defined(STM32F1)
        gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, pin.pin);
#else
        gpio_mode_setup(pin.port->which, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, pin.pin);
#endif
    }

    void set_pin_mode(const pin_type &pin, pin_mode::in_pulldown_t) {
        rcc_periph_clock_enable(pin.port->rcc);
#if defined(STM32F1)
        gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, pin.pin);
#else
        gpio_mode_setup(pin.port->which, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, pin.pin);
#endif
        write(pin, digital::low);
    }

    void write(const pin_type &pin, digital::high_t) {
        gpio_set(pin.port->which, pin.pin);
    }

    void write(const pin_type &pin, digital::low_t) {
        gpio_clear(pin.port->which, pin.pin);
    }

    bool read(const pin_type &pin) {
        return gpio_get(pin.port->which, pin.pin);
    }

private:
};
}

namespace tos_literals {
constexpr stm32::pin_t operator ""_pin(unsigned long long pin) {
    auto port_index = pin / 16;
    auto pin_index = pin % 16;
    uint16_t p = 1 << pin_index;
    return {&stm32::ports[port_index], p};
}
} // namespace tos_literals

stm32::gpio open_impl(tos::devs::gpio_t) {
    return {};
}
}