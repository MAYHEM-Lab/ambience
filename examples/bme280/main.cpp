//
// Created by fatih on 4/4/19.
//

#include <arch/drivers.hpp>
#include <tos/device/bme280.hpp>
#include <tos/expected.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/ft.hpp>

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (tos::stm32::ahb_clock / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i) {
        __asm__ __volatile__("nop");
    }
};

std::array<uint8_t, 2048> buf;
void bme_task() {
    using namespace tos::tos_literals;

    auto g = tos::open(tos::devs::gpio);

    tos::stm32::i2c t{tos::stm32::detail::i2cs[0], 22_pin, 23_pin};

    using namespace tos::device::bme280;
    driver b{{BME280_I2C_ADDR_PRIM}, &t, delay};
    b.set_config();

    tos::omemory_stream log(buf);
    tos::println(log, "Temperature, Pressure, Humidity");
    while (true) {
        using namespace std::chrono_literals;
        delay(70ms);
        with(
            b.read(components::all),
            [&](auto& comp_data) {
                tos::println(log,
                             int(comp_data.temperature),
                             int(comp_data.pressure),
                             int(comp_data.humidity));
            },
            [&](auto& err) { tos::println(log, "Error!", int(err)); });
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, bme_task);
}