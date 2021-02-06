#pragma once

namespace tos::elf {
enum class type : uint16_t {
    none = 0,
    relocatable = 1,
    executable = 2,
    dynamic = 3,
    core = 4,
};
}