//
// Created by fatih on 7/19/19.
//

#include "common/usart.hpp"

#include <arch/drivers.hpp>
#include <common/ble/gatt.hpp>
#include <tos/board.hpp>
#include <lidl/service.hpp>
#include <service_generated.hpp>
#include <tos/arm/assembly.hpp>
#include <tos/components/allocator.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/device/spbtlerf/adapter.hpp>
#include <tos/device/spbtlerf/gatt.hpp>
#include <tos/flags.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/print.hpp>
#include <tos/uuid.hpp>

class calc_impl : public tos::examples::calculator {
    int32_t multiply(const int32_t& a, const int32_t& b) override {
        return a * b;
    }
};

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
        tos::ble::characteristic_properties::write_with_response,
        255);

    if (!writeable_char) {
        LOG_ERROR("can't register attribute!");
        tos::this_thread::block_forever();
    }

    auto readable_char = force_get(service)->add_characteristic(
        readable_uuid,
        tos::util::set_flag(tos::ble::characteristic_properties::indicate,
            tos::ble::characteristic_properties::write_with_response),
        255);

    if (!readable_char) {
        LOG_ERROR("can't register attribute!");
        tos::this_thread::block_forever();
    }

    LOG("Registered");
    return force_get(service);
}

namespace tos {
void out_of_memory_handler() {
    while (true) {
        tos::arm::nop();
    }
}
}

void ble_task() {
    using namespace tos::tos_literals;
    using bs = tos::bsp::board_spec;

    auto g = tos::open(tos::devs::gpio);

    auto timer = open(tos::devs::timer<2>);
    tos::alarm alarm(&timer);
    auto erased_alarm = tos::erase_alarm(&alarm);

    auto usart = bs::default_com::open();

    tos::debug::serial_sink sink{&usart};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    tos::stm32::exti e;

    auto s = bs::ble::spi_dev::open();

    tos::device::spbtle::adapter_config conf;
    conf.alarm = erased_alarm.get();
    conf.m_irq_pin = tos::stm32::instantiate_pin(bs::ble::exti_pin);
    conf.gpio = &g;
    conf.cs_pin = tos::stm32::instantiate_pin(bs::ble::cs_pin);
    conf.exti = &e;
    conf.spi = &s;
    conf.reset_pin = tos::stm32::instantiate_pin(bs::ble::reset_pin);

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

    using namespace std::chrono_literals;

    auto on_disc = [&](int conn) {
        LOG("Disconnected:", conn);
        tos::launch(tos::alloc_stack, [&]{
      if (auto res = adv.start(100ms, "Tos BLE"); !res) {
          LOG_ERROR("Can't start advertising", int(force_error(res)));
      } else {
          LOG("Advertising");
      }
        });
    };
    bl->on_disconnect(tos::function_ref<void(int)>(on_disc));

    adv.start(100ms, "Tos BLE");
    LOG("disc started");

    calc_impl calc;
    auto runner = lidl::make_procedure_runner<tos::examples::calculator>();

    while (true) {
        received.down();
        std::array<uint8_t, 128> buf;
        lidl::message_builder mb(buf);
        runner(calc, echo, mb);
        readable.update_value(mb.get_buffer());
        LOG(tos::current_context().get_component<tos::allocator_component>()->allocator->in_use().value());
    }

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;

void tos_main() {
    tos::launch(sstore, ble_task);
}
