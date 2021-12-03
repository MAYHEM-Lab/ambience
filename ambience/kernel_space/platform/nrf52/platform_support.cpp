#include <block_memory_generated.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/ae/registry.hpp>
#include <tos/arm/core.hpp>
#include <arch/drivers.hpp>

tos::ae::registry_base& get_registry();

void platform_support::stage1_init() {
}

void platform_support::stage2_init() {
    tos::nrf52::gpio g;
    tos::arm::SCB::DEMCR.write(tos::arm::SCB::DEMCR.read() | 0x01000000);

    tos::arm::DWT::LAR.write(0xC5ACCE55);
    tos::arm::DWT::CYCCNT.write(0);
    tos::arm::DWT::CONTROL.write(tos::arm::DWT::CONTROL.read() | 1);

    tos::ae::preemptive_user_group_runner::create(
        tos::make_erased_preemptive_runner(*this));

    auto drv = new tos::device::bme280::driver(tos::bsp::board_spec::bme280::open());
    drv->set_config();
    drv->enable();
    auto wrapper = new tos::device::bme280::sensor_wrapper(drv);
    get_registry().register_service("weather_sensor", wrapper);
}

platform_group_args platform_support::make_args() {
    return platform_group_args{.trampoline = &*m_trampoline};
}