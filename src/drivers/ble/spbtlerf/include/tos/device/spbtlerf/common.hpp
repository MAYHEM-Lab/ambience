#pragma once

#include <cstdint>

namespace tos::device::spbtle {
struct fw_id {
    uint16_t build_number;
};

enum class errors
{
    unknown,
    timeout = 0xFF
};

class gatt_service;
class gatt_characteristic;
}