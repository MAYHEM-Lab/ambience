#pragma once

namespace tos::debug {
[[noreturn]] bool default_assert_handler(const char* msg);
}

#define Assert(x)              \
    ((static_cast<bool>(x)) || \
     ::tos::debug::default_assert_handler("Assertion failed " #x))
#define Expects(x) Assert(x)
#define Ensures(x) Assert(x)