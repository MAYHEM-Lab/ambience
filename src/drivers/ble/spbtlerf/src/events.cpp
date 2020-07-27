#include <bluenrg_aci_const.h>
#include <bluenrg_gatt_aci.h>
#include <hci_const.h>
#include <tos/device/spbtlerf/events.hpp>

namespace tos::device::spbtle {
struct evt_handler_impl {
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

    // Handles the confirmation of an indication from the client.
    void handle(const evt_gatt_server_confirmation& evt) {
        //TODO(fatih): how do we deliver this to the service that wrote the update?
        LOG("Received confirmation from", evt.conn_handle);
        for (auto& serv : m_services) {
            for (auto& attr : serv.characteristics()) {
                attr.on_indicate_response(evt.conn_handle);
            }
        }
    }

    void handle(const evt_blue_aci& evt) {
        switch (evt.ecode) {
        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
            handle(*reinterpret_cast<const evt_gatt_attr_modified_IDB05A1*>(evt.data));
            break;
        case EVT_BLUE_GATT_SERVER_CONFIRMATION_EVENT:
            handle(*reinterpret_cast<const evt_gatt_server_confirmation*>(evt.data));
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
        for (auto& serv : m_services) {
            for (auto& attr : serv.characteristics()) {
                attr.on_disconnect(evt.handle);
            }
        }
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

void hci_evt_handler::operator()(void* packet) {
    auto hci_pckt = static_cast<hci_uart_pckt*>(packet);
    m_impl->handle(*hci_pckt);
}

hci_evt_handler::hci_evt_handler()
    : m_impl(std::make_unique<evt_handler_impl>()) {
}

void hci_evt_handler::register_service(gatt_service& serv) {
    intrusive_ref(&serv);
    m_impl->m_services.push_back(serv);
}

void hci_evt_handler::remove_service(gatt_service& serv) {
    m_impl->m_services.erase(m_impl->m_services.unsafe_find(serv));
    intrusive_unref(&serv);
}
hci_evt_handler::~hci_evt_handler() = default;
} // namespace tos::device::spbtle