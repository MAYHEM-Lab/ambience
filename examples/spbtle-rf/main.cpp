//
// Created by fatih on 7/19/19.
//

#include "common/usart.hpp"

#include <SPBTLE_RF.h>
#include <arch/drivers.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/flags.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/print.hpp>
#include <tos/uuid.hpp>
#include <common/ble/gatt.hpp>

namespace tos::spbtle {
class gatt_service;

class gatt_characteristic : public list_node<gatt_characteristic> {
public:
    gatt_characteristic(gatt_service& service, uint16_t handle, int len);

    expected<void, errors> update_value(span<const uint8_t> data);

    uint16_t native_handle() const {
        return m_characteristic_handle;
    }

    int attributes_length() const {
        return m_len;
    }

    span<uint8_t> read_value(span<uint8_t> buf) const {
        uint16_t len = 0;
        auto res =
            aci_gatt_read_handle_value(native_handle() + 1, buf.size(), &len, buf.data());
        if (res != BLE_STATUS_SUCCESS) {
            LOG_ERROR("Couldn't read", res);
        }
        LOG("Len:", len);
        return buf.slice(0, len);
    }

private:
    friend class hci_evt_handler;

    void receive_modify(int connection, span<const uint8_t> data, int actual_attr) {
        LOG(connection,
            "wrote",
            data.size(),
            "bytes to",
            int(native_handle()),
            "(",
            actual_attr,
            ")");
    }

    intrusive_ptr<gatt_service> m_service;
    uint16_t m_characteristic_handle;
    int m_len;
};

class gatt_service : public list_node<gatt_service> {
public:
    gatt_service() = default;

    static expected<intrusive_ptr<gatt_service>, errors>
    create(const uuid& uuid, int max_chars, bool primary) {
        uint16_t serv_handle;
        auto ret = aci_gatt_add_serv(UUID_TYPE_128,
                                     uuid.id_,
                                     primary ? PRIMARY_SERVICE : SECONDARY_SERVICE,
                                     max_chars,
                                     &serv_handle);

        if (ret != BLE_STATUS_SUCCESS) {
            return unexpected(static_cast<errors>(ret));
        }

        auto res = make_intrusive<gatt_service>();
        res->m_service_handle = serv_handle;
        return res;
    }

    expected<gatt_characteristic*, errors>
    add_characteristic(const uuid& uuid, ble::characteristic_properties props, int len) {
        auto translate_props = [](ble::characteristic_properties props) {
            int res = 0;
            if (util::is_flag_set(props, ble::characteristic_properties::read)) {
                res |= CHAR_PROP_READ;
            }
            if (util::is_flag_set(
                    props, ble::characteristic_properties::write_without_response)) {
                res |= CHAR_PROP_WRITE_WITHOUT_RESP;
            }
            if (util::is_flag_set(props, ble::characteristic_properties::notify)) {
                res |= CHAR_PROP_NOTIFY;
            }
            return res;
        };

        uint16_t char_handle;
        auto ret = aci_gatt_add_char(m_service_handle,
                                     UUID_TYPE_128,
                                     uuid.id_,
                                     len,
                                     translate_props(props),
                                     ATTR_PERMISSION_NONE,
                                     GATT_NOTIFY_ATTRIBUTE_WRITE,
                                     16,
                                     true,
                                     &char_handle);

        if (ret != BLE_STATUS_SUCCESS) {
            return unexpected(static_cast<errors>(ret));
        }

        auto res = new gatt_characteristic(
            *this,
            char_handle,
            util::is_flag_set(props, ble::characteristic_properties::notify) ? 3 : 2);

        m_characteristics.push_back(*res);

        return res;
    }

    uint16_t native_handle() const {
        return m_service_handle;
    }

    const intrusive_list<gatt_characteristic>& characteristics() const {
        return m_characteristics;
    }

private:
    intrusive_list<gatt_characteristic> m_characteristics;
    uint16_t m_service_handle;

    int16_t m_refcnt;

    friend void intrusive_ref(gatt_service* serv) {
        serv->m_refcnt++;
    }

    friend void intrusive_unref(gatt_service* serv) {
        serv->m_refcnt--;
        if (serv->m_refcnt == 0) {
            delete serv;
        }
    }
};

gatt_characteristic::gatt_characteristic(gatt_service& service, uint16_t handle, int len)
    : m_service(&service)
    , m_characteristic_handle(handle)
    , m_len(len) {
}

expected<void, errors> gatt_characteristic::update_value(span<const uint8_t> data) {
    auto ret = aci_gatt_update_char_value(
        m_service->native_handle(), m_characteristic_handle, 0, data.size(), data.data());
    if (ret == BLE_STATUS_SUCCESS) {
        return {};
    }
    return unexpected(static_cast<errors>(ret));
}

/**
 * An instance of this class is used to handle events raised from the driver.
 */
class hci_evt_handler {
public:
    hci_evt_handler() {
        attach_HCI_CB(tos::function_ref<void(void*)>(*this));
    }

    void operator()(void* packet) {
        auto hci_pckt = static_cast<hci_uart_pckt*>(packet);
        handle(*hci_pckt);
    }

    void register_service(gatt_service& serv) {
        intrusive_ref(&serv);
        m_services.push_back(serv);
    }

    void remove_service(gatt_service& serv) {
        m_services.erase(m_services.unsafe_find(serv));
        intrusive_unref(&serv);
    }

private:
    intrusive_list<gatt_service> m_services;

    void handle(const evt_gatt_attr_modified_IDB05A1& evt) {
        bool handled = false;
        for (auto& serv : m_services) {
            for (auto& attr : serv.characteristics()) {
                if (attr.native_handle() <= evt.attr_handle &&
                    attr.native_handle() + attr.attributes_length() > evt.attr_handle) {
                    attr.receive_modify(
                        evt.conn_handle,
                        span<const uint8_t>{&evt.att_data[0], evt.data_length},
                        evt.attr_handle);
                    handled = true;
                }
            }
        }
        if (!handled) {
            LOG("Unhandled modified event for", evt.attr_handle);
        }
    }

    void handle(const evt_blue_aci& evt) {
        switch (evt.ecode) {
        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
            handle(*reinterpret_cast<const evt_gatt_attr_modified_IDB05A1*>(evt.data));
            break;
        default:
            LOG("Unhandled vendor event:", evt.ecode);
        }
    }

    void handle(const evt_conn_complete& evt) {
        LOG("Connected", int(evt.handle), int(evt.status));
    }

    void handle(const evt_disconn_complete& evt) {
        LOG("Disconnected", int(evt.handle), int(evt.status), int(evt.reason));
    }

    void handle(const evt_le_connection_complete& evt) {
        LOG("Connected", int(evt.handle), int(evt.status));
    }

    void handle(const evt_le_connection_update_complete& evt) {
        LOG("Update complete", int(evt.handle), int(evt.status));
    }

    void handle(const evt_le_meta_event& evt) {
        switch (evt.subevent) {
        case EVT_LE_CONN_COMPLETE:
            handle(*reinterpret_cast<const evt_le_connection_complete*>(&evt.data[0]));
            break;
        case EVT_LE_CONN_UPDATE_COMPLETE:
            handle(*reinterpret_cast<const evt_le_connection_update_complete*>(
                &evt.data[0]));
            break;
        default:
            LOG("Unhandled meta event", evt.subevent);
            break;
        }
    }

    void handle(const hci_event_pckt& packet) {
        switch (packet.evt) {
        case EVT_VENDOR:
            handle(*reinterpret_cast<const evt_blue_aci*>(&packet.data[0]));
            break;
        case EVT_CONN_COMPLETE:
            handle(*reinterpret_cast<const evt_conn_complete*>(&packet.data[0]));
            break;
        case EVT_DISCONN_COMPLETE:
            handle(*reinterpret_cast<const evt_disconn_complete*>(&packet.data[0]));
            break;
        case EVT_LE_META_EVENT:
            handle(*reinterpret_cast<const evt_le_meta_event*>(&packet.data[0]));
            break;
        default:
            LOG_TRACE("Unhandled event packet:", int(packet.evt));
        }
    }

    void handle(const hci_uart_pckt& packet) {
        if (packet.type != HCI_EVENT_PKT) {
            LOG_TRACE("Discard packet with type", int(packet.type));
            return;
        }

        handle(*reinterpret_cast<const hci_event_pckt*>(&packet.data[0]));
    }
};
} // namespace tos::spbtle

tos::intrusive_ptr<tos::spbtle::gatt_service> add_gatt_service() {
    static constexpr auto uuid =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E08");

    static constexpr auto uuid1 =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E09");

    static constexpr auto uuid2 =
        tos::uuid::from_canonical_string("025CA236-F196-23A4-304D-E8663F339E0A");

    auto service = tos::spbtle::gatt_service::create(uuid, 6, true);
    if (!service) {
        LOG_ERROR("can't create service!");
        tos::this_thread::block_forever();
    }

    auto writeable_char = force_get(service)->add_characteristic(
        uuid1,
        tos::util::set_flag(tos::ble::characteristic_properties::write_without_response,
                            tos::ble::characteristic_properties::read),
        255);

    if (!writeable_char) {
        LOG_ERROR("can't register attribute!");
        tos::this_thread::block_forever();
    }

    auto readable_char = force_get(service)->add_characteristic(
        uuid2,
        tos::util::set_flag(
            tos::util::set_flag(tos::ble::characteristic_properties::read,
                                tos::ble::characteristic_properties::notify),
            tos::ble::characteristic_properties::write_without_response),
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

    tos::spbtle::spbtle_rf bl(&s, &e, *erased_alarm, cs_pin, exti_pin, reset);

    LOG("began");

    while (true) {
        auto build_number = bl.get_fw_id();
        if (!build_number) {
            LOG_WARN("can't communicate", int(force_error(build_number)));
            continue;
        }
        LOG(bool(build_number), int(force_get(build_number).build_number));
        break;
    }

    auto gatt = bl.initialize_gatt();
    if (!gatt) {
        LOG_ERROR("gatt failed");
        tos::this_thread::block_forever();
    }

    auto gap = bl.initialize_gap(force_get(gatt), "Tos BLE");
    LOG("gap init'd");

    tos::spbtle::advertising adv;
    adv.start(1s, "Tos BLE");
    LOG("disc started");

    tos::spbtle::hci_evt_handler ev_handler;

    auto serv = add_gatt_service();
    auto& writeable =
        const_cast<tos::spbtle::gatt_characteristic&>(serv->characteristics().front());
    auto& readable =
        const_cast<tos::spbtle::gatt_characteristic&>(serv->characteristics().back());
    ev_handler.register_service(*serv);

    uint8_t buf[64];
    while (true) {
        tos::this_thread::sleep_for(alarm, 1s);

        auto ret = readable.update_value(tos::raw_cast(tos::span("Hello")));
        if (!ret) {
            LOG("Error while updating characteristic.\n");
        }

        tos::this_thread::sleep_for(alarm, 1s);

        ret = readable.update_value(tos::raw_cast(tos::span("World")));

        if (!ret) {
            LOG("Error while updating characteristic.\n");
        }

        if (!adv.start(1s, "Tos BLE")) {
        }
    }

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;

void tos_main() {
    tos::launch(sstore, ble_task);
}
