#pragma once

#include <cstring>
#include <tos/aarch64/assembly.hpp>
#include <tos/compiler.hpp>
#include <tos/span.hpp>

namespace tos::aarch64 {
class semihosting {
public:
    static constexpr bool unavailable() {
        return false;
    }

    static uint64_t clock() {
        return perform_call(0x10, 0);
    }

    static uint64_t elapsed() {
        uint64_t res;

        perform_call(0x30, reinterpret_cast<uint64_t>(&res));

        return res;
    }

    static void write0(const char* data) {
        perform_call(0x04, reinterpret_cast<uint64_t>(data));
    }

    [[noreturn]] static void exit(uint64_t code) {
        uint64_t args[2] = {0x20026, code};
        perform_call(0x18, reinterpret_cast<uint64_t>(&args));
        TOS_UNREACHABLE();
    }

private:
    static uint64_t perform_call(uint32_t opcode, uint64_t param);
};
} // namespace tos::aarch64