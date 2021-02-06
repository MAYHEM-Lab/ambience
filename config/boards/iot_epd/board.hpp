#pragma once

#include <l4_iot_disco/board.hpp>

namespace tos::bsp {
struct iot_epd_spec : l4_iot_disco_spec {
    struct epd {
        using spi_dev = spi1;
        static constexpr auto busy_pin = 3;   // D4
        static constexpr auto reset_pin = 20; // D5
        static constexpr auto dc_pin = 17;    // D6
        static constexpr auto cs_pin = 4;     // D7
    };
};
} // namespace tos::bsp