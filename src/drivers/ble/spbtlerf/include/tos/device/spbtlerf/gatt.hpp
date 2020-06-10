#pragma once

#include <common/ble/gatt.hpp>
#include <cstdint>
#include <tos/debug/log.hpp>
#include <tos/device/spbtlerf/common.hpp>
#include <tos/expected.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/span.hpp>
#include <tos/uuid.hpp>

namespace tos::device::spbtle {
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

    span<uint8_t> read_value(span<uint8_t> buf) const;

    void receive_modify(int connection, span<const uint8_t> data, int actual_attr);

private:
    intrusive_ptr<gatt_service> m_service;
    uint16_t m_characteristic_handle;
    int m_len;
};

class gatt_service : public list_node<gatt_service>, public ref_counted<gatt_service> {
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
