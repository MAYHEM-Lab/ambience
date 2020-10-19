#pragma once

#include <array>
#include <common/driver_base.hpp>
#include <common/gpio.hpp>
#include <cstdint>
#include <stm32_hal/gpio.hpp>
#include <stm32_hal/rcc_ex.hpp>
#include <stm32_hal/rcc.hpp>

namespace tos {
namespace stm32 {

struct pin_t {
    GPIO_TypeDef* port;
    uint16_t pin;
};

class gpio : public self_pointing<gpio>, public non_copy_movable {
public:
    using pin_type = pin_t;

    /**
     * Sets the given pin to be an output
     */
    void set_pin_mode(const pin_type& pin, pin_mode::output_t);

    /**
     * Sets the given pin to be an output, and configures it
     * to use the fastest IO speed as possible
     */
    void set_pin_mode(const pin_type& pin, pin_mode::output_t, pin_mode::fast_t);

    void set_pin_mode(const pin_type& pin, pin_mode::input_t);

    void set_pin_mode(const pin_type& pin, pin_mode::in_pullup_t);

    void set_pin_mode(const pin_type& pin, pin_mode::in_pulldown_t);

    void write(const pin_type& pin, digital::high_t) {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_SET);
    }

    void write(const pin_type& pin, digital::low_t) {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
    }

    bool read(const pin_type& pin) const {
        return HAL_GPIO_ReadPin(pin.port, pin.pin);
    }

private:
};
} // namespace stm32

inline stm32::gpio open_impl(tos::devs::gpio_t) {
    return {};
}
} // namespace tos

// IMPL

namespace tos::stm32 {
inline std::array<GPIO_TypeDef*, 9> ports = {GPIOA,
                                             GPIOB,
                                             GPIOC,
                                             GPIOD,
#ifdef GPIOE
                                             GPIOE,
#endif
#ifdef GPIOF
                                             GPIOF,
#endif
#ifdef GPIOG
                                             GPIOG,
#endif
#ifdef GPIOH
                                             GPIOH,
#endif
#ifdef GPIOI
                                             GPIOI
#endif
};

inline void enable_rcc(const GPIO_TypeDef* gpio) {
    if (gpio == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (gpio == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (gpio == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (gpio == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (gpio == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
#ifdef GPIOE
    } else if (gpio == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#ifdef GPIOF
    } else if (gpio == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#ifdef GPIOG
    } else if (gpio == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
#endif
#ifdef GPIOH
    } else if (gpio == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
#endif
#ifdef GPIOI
    } else if (gpio == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
#endif
    }
}

namespace detail {
struct gpio_speed {
    static constexpr auto high() {
        return GPIO_SPEED_FREQ_HIGH;
    }

    static constexpr auto medium() {
        return GPIO_SPEED_FREQ_MEDIUM;
    }

    static constexpr auto low() {
        return GPIO_SPEED_FREQ_LOW;
    }

    static constexpr auto highest() {
#if defined(GPIO_SPEED_FREQ_VERY_HIGH)
        return GPIO_SPEED_FREQ_VERY_HIGH;
#else
        return high();
#endif
    }
};
} // namespace detail

inline void tos::stm32::gpio::set_pin_mode(const pin_t& pin, pin_mode::output_t) {
    enable_rcc(pin.port);
    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = detail::gpio_speed::medium();
    HAL_GPIO_Init(pin.port, &init);
}

inline void
gpio::set_pin_mode(const gpio::pin_type& pin, pin_mode::output_t, pin_mode::fast_t) {
    enable_rcc(pin.port);

    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = detail::gpio_speed::highest();
    HAL_GPIO_Init(pin.port, &init);
}

inline void gpio::set_pin_mode(const gpio::pin_type& pin, pin_mode::input_t) {
    enable_rcc(pin.port);

    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_NOPULL;
    init.Speed = detail::gpio_speed::medium();
    HAL_GPIO_Init(pin.port, &init);
}

inline void gpio::set_pin_mode(const gpio::pin_type& pin, pin_mode::in_pullup_t) {
    enable_rcc(pin.port);

    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP;
    init.Speed = detail::gpio_speed::medium();
    HAL_GPIO_Init(pin.port, &init);
}

inline void gpio::set_pin_mode(const gpio::pin_type& pin, pin_mode::in_pulldown_t) {
    enable_rcc(pin.port);

    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLDOWN;
    init.Speed = detail::gpio_speed::medium();
    HAL_GPIO_Init(pin.port, &init);
}

inline pin_t instantiate_pin(int pin) {
    auto port_index = pin / 16;
    auto pin_index = pin % 16;
    uint16_t p = 1 << pin_index;
    return {stm32::ports[port_index], p};
}
} // namespace tos::stm32

namespace tos::tos_literals {
inline stm32::pin_t operator""_pin(unsigned long long pin) {
    return stm32::instantiate_pin(pin);
}
} // namespace tos::tos_literals