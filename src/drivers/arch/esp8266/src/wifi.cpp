//
// Created by fatih on 6/27/18.
//

#include <arch/wifi.hpp>
#include <string.h>
#include <tos/semaphore.hpp>
#include <tos/fixed_fifo.hpp>
#include <lwip/init.h>

extern "C"
{
#include <user_interface.h>
}

static tos::track_ptr<tos::esp82::wifi_connection> conn;
static tos::fixed_fifo<System_Event_t, 8> events;

static void wifi_handler(System_Event_t* ev)
{
    if (events.size() == events.capacity())
    {
        events.pop();
    }
    ets_printf("ev: %d\n", int(ev->event));
    events.push(*ev);
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}

static void scan_handler(void* arg, STATUS s)
{
    if (s == OK)
    {
        auto bssInfo = (bss_info *)arg;

        bssInfo = STAILQ_NEXT(bssInfo, next);

        while(bssInfo) {
            bssInfo = STAILQ_NEXT(bssInfo, next);
        }
    }
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}

namespace tos
{
    namespace esp82
    {
        expected<ipv4_addr_t, bool> wifi_connection::get_addr() {
            ip_info info;
            wifi_get_ip_info(STATION_IF, &info);

            ipv4_addr_t res;
            memcpy(res.addr, &info.ip, 4);

            return res;
        }

        wifi_connection::wifi_connection() {
            conn = get_ptr(*this);
        }

        wifi_connection::~wifi_connection() {
            if (discon)
            {
                wifi_station_disconnect();
            }
        }

        wifi::wifi() noexcept {
            wifi_set_event_handler_cb(wifi_handler);
            wifi_set_opmode_current(STATION_MODE);
        }

        wifi::~wifi() = default;

        void ICACHE_FLASH_ATTR wifi_connection::wait_for_dhcp() {
            while (!has_ip())
            {
                consume_event();
            }
        }

        void wifi_connection::consume_event(System_Event_t ev) {
            switch (ev.event)
            {
                case EVENT_STAMODE_GOT_IP:
                    m_state = states::operational;
                    break;
                case EVENT_STAMODE_DISCONNECTED:
                    discon = false;
                    m_state = states::disconnected;
                    break;
                case EVENT_STAMODE_CONNECTED:
                    discon = true;
                    m_state = states::waiting_dhcp;
                    break;
                default:
                    ets_printf("unexpected ev: %d\n", int(ev.event));
                    break;
            }
        }

        void wifi_connection::consume_all() {
            while (!events.empty())
            {
                consume_event(events.pop());
            }
        }

        void wifi_connection::consume_event() {
            consume_event(events.pop());
        }

        mac_addr_t wifi::get_ether_address() const {
            mac_addr_t res;
            wifi_get_macaddr(STATION_IF, res.addr);
            return res;
        }

        expected<wifi_connection, assoc_error>
        wifi::connect(tos::span<const char> ssid, tos::span<const char> passwd) noexcept {
            station_config stationConfig{};
            strncpy((char*)stationConfig.ssid, ssid.data(), 32);
            strncpy((char*)stationConfig.password, passwd.data(), 64);

            // clear the event queue
            while (!events.empty())
            {
                events.pop();
            }

            wifi_station_set_config_current(&stationConfig);
            wifi_station_connect();

            wifi_connection conn;

            while (!conn.is_connected())
            {
                conn.consume_event();
                if (conn.is_disconnected())
                {
                    return unexpected(assoc_error::unknown);
                }
            }

            lwip_init();
            return std::move(conn);
        }

        void wifi::scan() {
            wifi_station_scan(nullptr, scan_handler);
        }
    }
}