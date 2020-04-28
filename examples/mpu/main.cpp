//
// Created by fatih on 10/17/19.
//

#include "bitfield.hpp"
#include "tos/debug/debug.hpp"
#include "tos/expected.hpp"
#include "tos/thread.hpp"

#include <arch/drivers.hpp>
#include <optional>
#include <tos/barrier.hpp>
#include <tos/ft.hpp>
#include <tos/memory.hpp>
#include "mpu.hpp"

char foo[256];

void require_impl(bool expr, const char* /* str */) {
    if (!expr) {
        __BKPT(0);
    }
}

#define REQUIRE(expr) require_impl(bool(expr), #expr)

void mpu_task() {
    tos::cmsis::mpu mpu;
    auto regs = mpu.num_supported_regions();
    tos::debug::log("MPU Number of regions:", int(regs));

    auto min_size = mpu.min_region_size();
    tos::debug::log("MPU Minimum region size:", int(min_size));

    REQUIRE(!mpu.get_region(0));

    auto set_res =
        mpu.set_region(0, {.base = reinterpret_cast<uintptr_t>(&foo), .size = 128});
    REQUIRE(set_res);

    tos::debug::do_not_optimize(foo[128] = 5);
    tos::debug::do_not_optimize(foo[0] = 5);

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, mpu_task);
}
