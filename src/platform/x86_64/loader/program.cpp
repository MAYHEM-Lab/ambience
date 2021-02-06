#include <cstdint>
#include <tos/span.hpp>

namespace {
[[gnu::section(".to_load")]] uint8_t program[] = {
#include <program.data>
};
} // namespace

tos::span<const uint8_t> program_span{program};