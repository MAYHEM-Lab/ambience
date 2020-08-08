//
// Created by fatih on 7/19/19.
//

#include "common/usart.hpp"

#include <arch/drivers.hpp>
#include <common/ble/gatt.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/device/spbtlerf/adapter.hpp>
#include <tos/device/spbtlerf/gatt.hpp>
#include <tos/flags.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/print.hpp>
#include <tos/uuid.hpp>

tos::intrusive_ptr<tos::device::spbtle::gatt_service> add_gatt_service() {
    static constexpr auto uuid =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E08");

    static constexpr auto writeable_uuid =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E09");

    static constexpr auto readable_uuid =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E0A");

    auto service = tos::device::spbtle::gatt_service::create(uuid, 6, true);
    if (!service) {
        LOG_ERROR("can't create service!");
        tos::this_thread::block_forever();
    }

    auto writeable_char = force_get(service)->add_characteristic(
        writeable_uuid,
        tos::util::set_flag(tos::ble::characteristic_properties::write_with_response,
                            tos::ble::characteristic_properties::read),
        255);

    if (!writeable_char) {
        LOG_ERROR("can't register attribute!");
        tos::this_thread::block_forever();
    }

    auto readable_char = force_get(service)->add_characteristic(
        readable_uuid,
        tos::util::set_flag(
            tos::util::set_flag(tos::ble::characteristic_properties::read,
                                tos::ble::characteristic_properties::indicate),
            tos::ble::characteristic_properties::write_with_response),
        255);

    if (!readable_char) {
        LOG_ERROR("can't register attribute!");
        tos::this_thread::block_forever();
    }

    LOG("Registered");
    return force_get(service);
}

void ble_task() {
    using namespace tos::tos_literals;
    auto reset = 8_pin;
    auto cs_pin = 61_pin;
    auto exti_pin = 70_pin;

    auto g = tos::open(tos::devs::gpio);

    auto timer = open(tos::devs::timer<2>);
    tos::alarm alarm(&timer);
    auto erased_alarm = tos::erase_alarm(&alarm);

    auto usart =
        tos::open(tos::devs::usart<1>, tos::uart::default_115200, 23_pin, 22_pin);
    using namespace std::chrono_literals;

    tos::debug::serial_sink sink{&usart};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    tos::stm32::exti e;

    tos::stm32::spi s(tos::stm32::detail::spis[2], 42_pin, 43_pin, 44_pin);
    // s.set_8_bit_mode();

    tos::device::spbtle::adapter_config conf;
    conf.alarm = erased_alarm.get();
    conf.m_irq_pin = exti_pin;
    conf.gpio = &g;
    conf.cs_pin = cs_pin;
    conf.exti = &e;
    conf.spi = &s;
    conf.reset_pin = reset;

    auto bl = force_get(tos::device::spbtle::adapter::open(conf));

    LOG("began");

    while (true) {
        auto build_number = bl->get_fw_id();
        if (!build_number) {
            LOG_ERROR("can't communicate", int(force_error(build_number)));
            continue;
        }
        LOG(bool(build_number), int(force_get(build_number).build_number));
        break;
    }


    auto gatt = bl->initialize_gatt();
    if (!gatt) {
        LOG_ERROR("gatt failed");
        tos::this_thread::block_forever();
    }

    auto gap = bl->initialize_gap(force_get(gatt), "Tos BLE");
    LOG("gap init'd");

    auto serv = add_gatt_service();
    auto& writeable = const_cast<tos::device::spbtle::gatt_characteristic&>(
        serv->characteristics().front());
    auto& readable = const_cast<tos::device::spbtle::gatt_characteristic&>(
        serv->characteristics().back());

    std::vector<uint8_t> echo;
    tos::semaphore received{0};
    auto write_cb = [&](int, tos::span<const uint8_t> data) {
        echo = std::vector<uint8_t>(data.begin(), data.end());
        received.up();
        LOG("Received data:", data);
    };

    writeable.set_modify_callback(
        tos::function_ref<void(int, tos::span<const uint8_t>)>(write_cb));

    tos::device::spbtle::advertising adv;
    adv.start(1s, "Tos BLE");
    LOG("disc started");

    while (true) {
        received.down();
        readable.update_value(echo);
        if (!adv.start(1s, "Tos BLE")) {
        }
    }

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;

void tos_main() {
    tos::launch(sstore, ble_task);
}
