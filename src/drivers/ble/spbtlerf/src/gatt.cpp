extern "C" {
#include <bluenrg_gatt_aci.h>
}
#include <tos/device/spbtlerf/adapter.hpp>
#include <tos/device/spbtlerf/gatt.hpp>
#include <tos/flags.hpp>

namespace tos::device::spbtle {
gatt_characteristic::gatt_characteristic(gatt_service& service,
                                         uint16_t handle,
                                         int len,
                                         bool indicate)
    : m_service(&service)
    , m_characteristic_handle(handle)
    , m_len(len) {
    if (indicate) {
        m_indication = std::make_unique<indicate_data>();
    }
}

expected<void, errors> gatt_characteristic::update_value(span<const uint8_t> data) {
    auto ret = aci_gatt_update_char_value(
        m_service->native_handle(), m_characteristic_handle, 0, data.size(), data.data());

    if (ret == BLE_STATUS_SUCCESS) {
        if (m_indication) {
            lock_guard lg{m_indication->protect};
            LOG("Should wait for indicate response here.");
            for (auto conn : m_indication->enabled_connections) {
                LOG(conn);
                m_indication->wait.down();
            }
        }
        return {};
    }

    return unexpected(static_cast<errors>(ret));
}

span<uint8_t> gatt_characteristic::read_value(span<uint8_t> buf) const {
    uint16_t len = 0;
    auto res =
        aci_gatt_read_handle_value(native_handle() + 1, buf.size(), &len, buf.data());
    if (res != BLE_STATUS_SUCCESS) {
        LOG_ERROR("Couldn't read", res);
    }
    LOG("Len:", len);
    return buf.slice(0, len);
}

void gatt_characteristic::receive_modify(int connection,
                                         span<const uint8_t> data,
                                         int actual_attr) {
    LOG(connection,
        "wrote",
        data.size(),
        "bytes to",
        int(native_handle()),
        "(",
        actual_attr,
        ")");

    if (actual_attr == native_handle() + 2) {
        switch (data[0]) {
        case 0x00:
            if (m_indication) {
                m_indication->enabled_connections.erase(
                    std::remove(m_indication->enabled_connections.begin(),
                                m_indication->enabled_connections.end(),
                                connection));
            }
            LOG("Disabled indications/notifications");
            break;
        case 0x01:
            LOG("Enabled notifications");
            break;
        case 0x02: {
            // Enabled indications
            LOG("Enabled indications");
            if (!m_indication) {
                LOG_WARN("Connection tried to enable indications on a characteristic "
                         "without indications");
                break;
            }
            lock_guard lg{m_indication->protect};
            m_indication->enabled_connections.push_back(connection);
        } break;
        }
    }

    m_on_modify(connection, data);
}

void gatt_characteristic::on_indicate_response(int connection) {
    if (!m_indication) {
        return;
    }
    m_indication->wait.up();
}

void gatt_characteristic::on_disconnect(int connection) {
    if (!m_indication) {
        return;
    }
    lock_guard lg{m_indication->protect};
    m_indication->enabled_connections.erase(
        std::remove(m_indication->enabled_connections.begin(),
                    m_indication->enabled_connections.end(),
                    connection));
}

expected<intrusive_ptr<gatt_service>, errors>
gatt_service::create(const uuid& uuid, int max_chars, bool primary) {
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

    adapter::instance()->service_created(*res);

    return res;
}

namespace {
auto translate_props = [](ble::characteristic_properties props) {
    int res = 0;
    if (util::is_flag_set(props, ble::characteristic_properties::read)) {
        res |= CHAR_PROP_READ;
    }
    if (util::is_flag_set(props,
                          ble::characteristic_properties::write_without_response)) {
        res |= CHAR_PROP_WRITE_WITHOUT_RESP;
    }
    if (util::is_flag_set(props, ble::characteristic_properties::write_with_response)) {
        res |= CHAR_PROP_WRITE;
    }
    if (util::is_flag_set(props, ble::characteristic_properties::notify)) {
        res |= CHAR_PROP_NOTIFY;
    }
    if (util::is_flag_set(props, ble::characteristic_properties::indicate)) {
        res |= CHAR_PROP_INDICATE;
    }
    return res;
};
}

expected<gatt_characteristic*, errors> gatt_service::add_characteristic(
    const uuid& uuid, ble::characteristic_properties props, int len) {
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
        util::is_flag_set(props, ble::characteristic_properties::notify) ||
                util::is_flag_set(props, ble::characteristic_properties::indicate)
            ? 3
            : 2,
        util::is_flag_set(props, ble::characteristic_properties::indicate));

    m_characteristics.push_back(*res);

    return res;
}

expected<gatt_characteristic*, errors> gatt_service::add_characteristic(
    uint16_t short_uuid, ble::characteristic_properties props, int len) {
    uint16_t char_handle;
    auto ret = aci_gatt_add_char(m_service_handle,
                                 UUID_TYPE_16,
                                 reinterpret_cast<const uint8_t*>(&short_uuid),
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
        util::is_flag_set(props, ble::characteristic_properties::notify) ||
                util::is_flag_set(props, ble::characteristic_properties::indicate)
            ? 3
            : 2,
        util::is_flag_set(props, ble::characteristic_properties::indicate));

    m_characteristics.push_back(*res);

    return res;
}
} // namespace tos::device::spbtle