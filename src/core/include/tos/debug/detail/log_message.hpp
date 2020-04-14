#pragma once

#include <cstdint>
#include <string_view>
#include <variant>

namespace tos::debug::detail {
using log_elem = std::variant<std::string_view, int64_t, uint64_t, double>;

struct log_message {
    uint64_t timestamp;
};
}