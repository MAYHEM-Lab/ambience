#pragma once

#include <arch/mailbox.hpp>

namespace tos::raspi3 {
inline std::array<uint8_t, 8> get_board_serial() {
    property_channel_tags_builder builder;
    auto buf = builder.add(0x00010004, {0, 0}).end();
    property_channel property;
    if (!property.transaction(buf)) {
        tos::debug::panic("can't set clock speed");
    }
    std::array<uint8_t, 8> res;
    memcpy(res.data(), &buf[5], 4);
    memcpy(res.data() + 4, &buf[6], 4);
    return res;
}
}