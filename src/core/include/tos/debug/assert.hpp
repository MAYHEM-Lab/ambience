#pragma once

#include <tos/debug/panic.hpp>

namespace tos::debug {
[[noreturn]]
inline bool default_assert_handler(const char* msg) {
    panic(msg);
}
}

#define Assert(x) ((static_cast<bool>(x)) || ::tos::debug::default_assert_handler("Assertion failed " #x))
#define Expects(x)
#define Ensures(x)