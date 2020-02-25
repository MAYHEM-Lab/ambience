#pragma once

#include "detail/bcm2837.hpp"
#include <tos/span.hpp>
#include <tos/self_pointing.hpp>

namespace tos::raspi3 {
class uart0 : public self_pointing<uart0> {
public:
    uart0();

    int write(tos::span<const uint8_t>);
    span<uint8_t> read(tos::span<uint8_t>);
private:
};
}
