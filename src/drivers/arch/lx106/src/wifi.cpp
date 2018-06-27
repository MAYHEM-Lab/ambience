//
// Created by fatih on 6/27/18.
//

#include <wifi.hpp>
#include <string.h>
#include <tos/semaphore.hpp>
#include <util/include/tos/fixed_fifo.hpp>

extern "C"
{
#include <user_interface.h>
}

static tos::fixed_fifo<System_Event_t, 8> events;

static void wifi_handler(System_Event_t* ev)
{
    if (events.size() == events.capacity())
    {
        events.pop();
    }
    events.push(*ev);
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}

static void scan_handler(void* arg, STATUS s)
{
    if (s == OK)
    {
        auto bssInfo = (bss_info *)arg;

        bssInfo = STAILQ_NEXT(bssInfo, next);

        int i = 0;

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
        ipv4_addr wifi_connection::get_addr() {
            ip_info info;
            wifi_get_ip_info(STATION_IF, &info);
            ipv4_addr res;
            memcpy(res.addr, &info.ip, 4);
            return res;
        }

        wifi::wifi() noexcept {
            wifi_set_event_handler_cb(wifi_handler);
            wifi_set_opmode_current(STATION_MODE);
        }

        bool wifi::connect(tos::span<const char> ssid, tos::span<const char> passwd) noexcept {
            station_config stationConfig{};
            strncpy((char*)stationConfig.ssid, ssid.data(), 32);
            strncpy((char*)stationConfig.password, passwd.data(), 64);

            while (events.size() > 0)
            {
                events.pop();
            }

            wifi_station_set_config_current(&stationConfig);
            wifi_station_connect();

            while (true)
            {
                auto ev = events.pop();
                switch (ev.event)
                {
                    case EVENT_STAMODE_DISCONNECTED:
                        wifi_station_disconnect();
                        return false;
                    case EVENT_STAMODE_CONNECTED:
                        return true;
                    default:
                        break;
                }
            }
        }

        bool wifi::wait_for_dhcp() {
            auto ev = events.pop();
            if (ev.event == EVENT_STAMODE_GOT_IP)
            {
                return true;
            }
            return false;
        }

        void wifi::scan() {
            wifi_station_scan(nullptr, scan_handler);
        }
    }
}