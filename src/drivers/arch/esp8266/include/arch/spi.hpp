//
// Created by Mehmet Fatih BAKIR on 08/10/2018.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <tos/span.hpp>

namespace tos {
namespace esp82 {
class spi : public self_pointing<spi> {
public:
    using gpio_type = esp82::gpio;

    explicit spi(gpio_type&);

    uint8_t exchange(uint8_t byte);
    void exchange_many(tos::span<uint8_t> buffer);
};
} // namespace esp82
} // namespace tos