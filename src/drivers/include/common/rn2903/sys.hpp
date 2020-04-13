//
// Created by fatih on 12/11/18.
//

#pragma once

#include "rn2903_common.hpp"

#include <chrono>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/print.hpp>

namespace tos {
namespace rn2903 {
template<class UartT, class AlarmT>
constexpr tos::expected<uint8_t, failures>
nvm_get(nvm_addr_t addr,
        UartT& uart,
        AlarmT& alarm,
        std::chrono::milliseconds to = std::chrono::seconds{5}) {
    tos::println(uart, "sys get nvm", int(addr.addr));
    uint8_t buf[20]{};
    using namespace std::chrono_literals;
    auto r = uart.read(buf, alarm, to);

    if (r.size() == 0) {
        return unexpected(failures::timeout);
    }

    if (r.size() > 4) {
        return unexpected(failures::invalid_param);
    }

    buf[2] = 0;

    return strtoul(reinterpret_cast<const char*>(buf), nullptr, 16);
}
} // namespace rn2903
} // namespace tos