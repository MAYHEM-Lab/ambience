//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/tty.hpp>
#include <common/usart.hpp>
#include <tos/span.hpp>

#undef putc

namespace tos {
namespace x86 {
class stdio : public self_pointing<stdio> {
public:
    static int write(span<const uint8_t> buf);
    static span<uint8_t> read(span<uint8_t> buf);
};
} // namespace x86

template<class T>
inline x86::stdio open_impl(devs::usart_t<0>, T) {
    return {};
}

inline x86::stdio open_impl(devs::tty_t<0>) {
    return {};
}
} // namespace tos
