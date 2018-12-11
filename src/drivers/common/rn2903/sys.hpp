//
// Created by fatih on 12/11/18.
//

#pragma once

#include <cstdint>
#include <tos/print.hpp>
#include <tos/expected.hpp>

namespace tos
{
    namespace rn2903
    {
        enum class failures
        {
            timeout,
            invalid_param
        };

        template <class UartT, class AlarmT>
        constexpr tos::expected<uint8_t, failures>
        nvm_get(uint16_t addr, UartT& uart, AlarmT& alarm)
        {
            tos::println(uart, "sys get nvm", int(addr));
            char buf[20] {};
            using namespace std::chrono_literals;
            auto r = uart.read(buf, alarm, 5s);

            if (r.size() == 0)
            {
                return unexpected(failures::timeout);
            }

            if (r.size() > 4)
            {
                return unexpected(failures::invalid_param);
            }

            buf[2] = 0;

            return strtoul(buf, nullptr, 16);
        }
    } // namespace rn2903
} // namespace tos