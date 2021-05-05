#pragma once

#include <tos/arm/core.hpp>
#include <chrono>
#include <tos/self_pointing.hpp>

namespace tos::arm {
struct cyccnt_clock : self_pointing<cyccnt_clock> {
public:
    cyccnt_clock() {
        // Enable CoreDebug_DEMCR_TRCENA_Msk
        tos::arm::SCB::DEMCR.write(tos::arm::SCB::DEMCR.read() | 0x01000000);

        tos::arm::DWT::CYCCNT.write(0);
        tos::arm::DWT::CONTROL.write(tos::arm::DWT::CONTROL.read() | 1);
    }

    typedef std::ratio<1, 80'000'000> clock_cycle;

    using rep = uint64_t;
    using period = clock_cycle;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<cyccnt_clock>;

    [[nodiscard]] time_point now() const {
        return time_point(duration(tos::arm::DWT::CYCCNT.read()));
    }

private:
};
} // namespace tos::arm