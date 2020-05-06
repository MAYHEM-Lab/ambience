#pragma once

#include <tos/debug/panic.hpp>

namespace tos::debug {
[[noreturn]]
inline bool default_assert_handler() {
    panic("Assertion failed");
}
}

#define Assert(x) ((x) || ::tos::debug::default_assert_handler())
#define Expects(x)
#define Ensures(x)