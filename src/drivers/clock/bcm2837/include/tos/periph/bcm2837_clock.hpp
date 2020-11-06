#pragma once

#include <tos/debug/panic.hpp>
#include <arch/mailbox.hpp>

namespace tos::periph {
class clock_manager {
public:
    uint32_t get_max_frequency(bcm283x::clocks clock) {
        raspi3::property_channel props;

        raspi3::property_channel_tags_builder builder;
        auto buf = builder
            .add(static_cast<uint32_t>(bcm283x::clock_tags::get_max_clock_rate),
                 {static_cast<uint32_t>(clock), 0})
            .end();

        auto res = props.transaction(buf);
        Assert(res && "Property transaction failed");

        Assert(buf[1] != 0 && "bad response");

        auto code = buf[1] - 0x80000000;
        Assert(code == 0 && "error");

        return buf[6];
    }

    uint32_t get_frequency(bcm283x::clocks clock) {
        raspi3::property_channel props;

        raspi3::property_channel_tags_builder builder;
        auto buf = builder
            .add(static_cast<uint32_t>(bcm283x::clock_tags::get_clock_rate),
                 {static_cast<uint32_t>(clock), 0})
            .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }

        return buf[6];
    }

    void set_frequency(bcm283x::clocks clock, uint32_t hertz) {
        raspi3::property_channel props;

        raspi3::property_channel_tags_builder builder;
        auto buf = builder
            .add(static_cast<uint32_t>(bcm283x::clock_tags::set_clock_rate),
                 {static_cast<uint32_t>(clock), hertz, 0})
            .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }
    }

private:
};
}