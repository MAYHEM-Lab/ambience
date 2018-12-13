//
// Created by fatih on 12/11/18.
//

#pragma once

#include <tos/expected.hpp>
#include "rn2903_common.hpp"
#include <chrono>

namespace tos
{
namespace rn2903
{
    template <class UartT, class AlarmT>
    constexpr expected<join_res, failures>
    join_otaa(UartT& uart, AlarmT& alarm)
    {
        using namespace std::chrono_literals;
        tos::println(uart, "mac join otaa");

        char buf[20] {};
        using namespace std::chrono_literals;
        auto r = uart.read(buf, alarm, 5s);

        if (r.size() < 4) // ok message is the shortest, and have 4 chars with cr lf
        {
            return unexpected(failures::timeout);
        }

        if (r.size() > 4) {
            switch (r[0])
            {
                case 'i': return unexpected(failures::invalid_param);
                case 'k': return unexpected(failures::keys_not_init);
                case 'n': return unexpected(failures::no_free_ch);
                case 's': return unexpected(failures::silent);
                case 'b': return unexpected(failures::busy);
                case 'm': return unexpected(failures::mac_paused);
            }
        }

        // should be ok
        // assert(r[0] == 'o' && r[1] == 'k');

        // the first reply comes pretty fast, but we'll have to wait for
        // the second one

        r = uart.read(buf, alarm, 15s);

        if (r.size() < 8) // denied is the shortest message, and have 8 chars
        {
            return unexpected(failures::timeout);
        }

        return r[0] == 'a' ? join_res::joined : join_res::denied;
    }

} // namespace rn2903
} // namespace tos