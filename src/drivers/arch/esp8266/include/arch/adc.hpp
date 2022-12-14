//
// Created by fatih on 11/7/18.
//

#pragma once

extern "C" {
#include <user_interface.h>
}

namespace tos {
namespace esp82 {
class adc {
public:
    uint16_t ICACHE_FLASH_ATTR read() const {
        return system_adc_read();
    }
};
} // namespace esp82
} // namespace tos