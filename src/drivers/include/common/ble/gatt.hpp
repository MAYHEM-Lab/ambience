#pragma once

namespace tos::ble {
enum class characteristic_properties
{
    read = 1,
    write = 2,
    write_with_response = write,
    write_without_response = 4,
    notify = 8,
    indicate = 16
};
}
