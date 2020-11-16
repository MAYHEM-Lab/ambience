#include <tos/debug/assert.hpp>
#include <tos/debug/panic.hpp>

namespace tos::debug {
[[noreturn]]
bool default_assert_handler(const char* msg) {
    panic(msg);
}
}