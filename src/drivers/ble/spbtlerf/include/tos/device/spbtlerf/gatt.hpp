#pragma once

#include <common/ble/gatt.hpp>
#include <cstdint>
#include <tos/debug/log.hpp>
#include <tos/device/spbtlerf/common.hpp>
#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>
#include <tos/uuid.hpp>

namespace tos::device::spbtle {
class gatt_characteristic : public list_node<gatt_characteristic> {
public:
    gatt_characteristic(gatt_service& service, uint16_t handle, int len, bool indicate);

    // Updates the value of this characteristic.
    // Will block if there are any connections that enabled indications until all
    // indication responses are received.
    expected<void, errors> update_value(span<const uint8_t> data);

    // Reads the current value of the characteristic.
    span<uint8_t> read_value(span<uint8_t> buf) const;

    void set_modify_callback(tos::function_ref<void(int, span<const uint8_t>)> cb) {
        m_on_modify = cb;
    }

    // The following functions are called by the events subsystem and are considered
    // private, do not call them!
    void receive_modify(int connection, span<const uint8_t> data, int actual_attr);
    void on_indicate_response(int connection);
    void on_disconnect(int connection);

    uint16_t native_handle() const {
        return m_characteristic_handle;
    }

    int attributes_length() const {
        return m_len;
    }

private:
    intrusive_ptr<gatt_service> m_service;
    uint16_t m_characteristic_handle;
    // Data length of the characteristic.
    int m_len;

    // This function will be called when a client modifies this characteristic.
    tos::function_ref<void(int, span<const uint8_t>)> m_on_modify{
        [](int, span<const uint8_t>, void*) {}};

    struct indicate_data {
        mutex protect;
        semaphore wait{0};

        // This should be a set, preferably from ETL.
        std::vector<uint16_t> enabled_connections;
    };

    std::unique_ptr<indicate_data> m_indication;
};

class gatt_service
    : public list_node<gatt_service>
    , public ref_counted<gatt_service> {
public:
    gatt_service() = default;

    static expected<intrusive_ptr<gatt_service>, errors>
    create(const uuid& uuid, int max_chars, bool primary);

    expected<gatt_characteristic*, errors>
    add_characteristic(const uuid& uuid, ble::characteristic_properties props, int len);

    uint16_t native_handle() const {
        return m_service_handle;
    }

    const intrusive_list<gatt_characteristic>& characteristics() const {
        return m_characteristics;
    }

private:
    intrusive_list<gatt_characteristic> m_characteristics;
    uint16_t m_service_handle;
};
} // namespace tos::device::spbtle
