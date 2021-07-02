#pragma once

#include "common/spi.hpp"
#include <arch/drivers.hpp>

namespace tos::bsp {
struct l4_iot_disco_spec {
    static constexpr auto& name() {
        return "STM32 IoT Demo Board";
    }

    static constexpr auto led_pin = 5;

    struct usart1 {
        static constexpr auto tag = devs::usart<1>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 22;
        static constexpr auto rx_pin = 23;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(rx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    using default_com = usart1;

    struct spi1 {
        static constexpr auto tag = devs::spi<1>;
        static constexpr auto clk_pin = 5;
        static constexpr auto miso_pin = 6;
        static constexpr auto mosi_pin = 7;

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin));
        }
    };

    struct spi1s {
        static constexpr auto tag = devs::spi<1>;
        static constexpr auto clk_pin = 5;
        static constexpr auto miso_pin = 6;
        static constexpr auto mosi_pin = 7;

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin),
                             tos::spi_mode::slave);
        }
    };

    struct spi3 {
        static constexpr auto tag = devs::spi<3>;
        static constexpr auto clk_pin = 42;
        static constexpr auto miso_pin = 43;
        static constexpr auto mosi_pin = 44;

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin));
        }
    };

    struct ble {
        using spi_dev = spi3;
        static constexpr auto exti_pin = 70;
        static constexpr auto cs_pin = 61;
        static constexpr auto reset_pin = 8;
    };

    struct i2c2 {
        static constexpr auto tag = devs::i2c<2>;
        static constexpr auto scl_pin = 26;
        static constexpr auto sda_pin = 27;

        static auto open() {
            return tos::open(tag,
                             tos::i2c_type::master,
                             stm32::instantiate_pin(scl_pin),
                             stm32::instantiate_pin(sda_pin));
        }
    };

    struct hts221 {
        using i2c_dev = i2c2;
        static constexpr auto address = 0x5F;
    };

    struct flash {
        static auto open() {
            return stm32::flash{};
        }
    };
};
} // namespace tos::bsp