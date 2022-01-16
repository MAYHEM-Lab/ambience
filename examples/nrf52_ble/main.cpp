#include "arch/timer.hpp"
#include "common/alarm.hpp"
#include "common/inet/tcp_ip.hpp"
#include "controller/ble_ll_adv.h"
#include "host/ble_hs_id.h"
#include "host/ble_uuid.h"
#include "nimble/nimble_port.h"
#include "tos/debug/debug.hpp"
#include "tos/interrupt.hpp"
#include "tos/self_pointing.hpp"
#include "tos/semaphore.hpp"
#include "tos/stack_storage.hpp"
#include "tos/thread.hpp"
#include <cmath>
#include <etl/list.h>
#include <host/ble_gatt.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <nimble/ble.h>
#include <nrfx_clock.h>
#include <random>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <stdarg.h>
#include <stdio.h>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;

extern "C" {
int printf(const char* fmt, ...) {
    static char buf[1024];

    va_list myargs;
    va_start(myargs, fmt);
    auto ret = vsnprintf(buf, std::size(buf), fmt, myargs);
    tos::debug::log(buf);
    va_end(myargs);

    return ret;
}
}

struct itm : tos::self_pointing<itm> {
    itm() {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

        NRF_CLOCK->TRACECONFIG =
            (NRF_CLOCK->TRACECONFIG & ~CLOCK_TRACECONFIG_TRACEPORTSPEED_Msk) |
            (CLOCK_TRACECONFIG_TRACEPORTSPEED_16MHz
             << CLOCK_TRACECONFIG_TRACEPORTSPEED_Pos);

        NRF_CLOCK->TRACECONFIG |= CLOCK_TRACECONFIG_TRACEMUX_Serial
                                  << CLOCK_TRACECONFIG_TRACEMUX_Pos;

        NRF_P1->PIN_CNF[0] = (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) |
                             (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                             (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);

        ITM->TCR |= 1;
        ITM->TER |= 1;
    }

    int write(tos::span<const uint8_t> data) {
        tos::int_guard ig;
        for (auto c : data) {
            ITM_SendChar(c);
        }
        return data.size();
    }
};

namespace tos::nimble {
extern tos::any_alarm* alarm;
}
static uint8_t g_own_addr_type;
static const char* device_name = "Tos";
static uint16_t conn_handle;

/* adv_event() calls advertise(), so forward declaration is required */
static void advertise(void);

tos::semaphore log_sem{0};
etl::list<std::string, 100> log_strs;

void async_log_task() {
    while (true) {
        log_sem.down();
        auto str = std::move(log_strs.front());
        log_strs.pop_front();
        LOG(str);
    }
}

tos::semaphore adv_sem{0};
static int adv_event(struct ble_gap_event* event, void* arg) {
    switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
        // LOG_FORMAT("Advertising completed, termination code: {}",
        //            event->adv_complete.reason);
        adv_sem.up();
        // advertise();
        break;
    case BLE_GAP_EVENT_CONNECT:
        assert(event->connect.status == 0);
        LOG_FORMAT("connection {}; status={}; {}",
                   event->connect.status == 0 ? "established" : "failed",
                   event->connect.status,
                   event->connect.conn_handle);
        conn_handle = event->connect.conn_handle;
        break;
    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        /* connected device requests update of connection parameters,
           and these are being filled in - NULL sets default values */
        LOG("updating conncetion parameters...");
        event->conn_update_req.conn_handle = conn_handle;
        event->conn_update_req.peer_params = NULL;
        LOG("connection parameters updated!");
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        LOG_FORMAT("disconnect; reason={}", event->disconnect.reason);

        /* reset conn_handle */
        conn_handle = BLE_HS_CONN_HANDLE_NONE;

        /* Connection terminated; resume advertising */
        advertise();
        break;
    default:
        LOG_ERROR("Advertising event not handled,"
                  "event code:",
                  event->type);
        break;
    }
    return 0;
}

static void advertise(void) {
    /* set adv parameters */
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    /* fill all fields and parameters with zeros */
    memset(&adv_params, 0, sizeof(adv_params));
    memset(&fields, 0, sizeof(fields));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    auto name = ble_svc_gap_device_name();
    fields.name = (uint8_t*)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    auto rc = ble_gap_adv_set_fields(&fields);
    LOG(rc);
    rc = ble_gap_adv_start(g_own_addr_type, NULL, 1000, &adv_params, adv_event, NULL);
    LOG(rc);
}

void entry() {
    auto out = itm{};
    // auto out = bs::default_com::open();
    tos::debug::serial_sink uart_sink(&out);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::info);
    tos::debug::set_default_log(&uart_log);

    auto tim = tos::nrf52::timer0{1};
    auto alarm = tos::erase_alarm(tos::alarm{&tim});
    tos::nimble::alarm = alarm.get();

    using namespace std::chrono_literals;
    tos::this_thread::sleep_for(*alarm, 5s);

    tos::launch(tos::alloc_stack, async_log_task);

    LOG("Booted!!");

    static tos::semaphore clk_sem{0};

    nrfx_clock_init([](nrfx_clock_evt_type_t evt) {
        if (evt == NRFX_CLOCK_EVT_LFCLK_STARTED) {
            LOG(evt);
            clk_sem.up_isr();
        }
    });

    nrfx_clock_enable();
    nrfx_clock_lfclk_start();

    clk_sem.down();

    // NRF_CLOCK->TASKS_LFCLKSTOP = 1;
    // NRF_CLOCK->LFCLKSRC = 0;
    // NRF_CLOCK->TASKS_LFCLKSTART = 1;

    nimble_port_init();
    LOG("Init done");

    ble_svc_gap_init();
    ble_svc_gatt_init();

    static tos::stack_storage<> ll_stak, hs_stak;
    tos::launch(ll_stak, nimble_port_ll_task_func, nullptr);
    tos::launch(hs_stak, nimble_port_run);

    static std::atomic<bool> sync = false;

    ble_hs_cfg.sync_cb = [] {
        LOG("Sync callback");
        ble_addr_t addr;

        /* generate new non-resolvable private address */

        auto rc = ble_hs_id_gen_rnd(1, &addr);
        LOG(rc);

        /* set generated address */

        rc = ble_hs_id_set_rnd(addr.val);
        LOG(rc);

        rc = ble_hs_util_ensure_addr(0);
        LOG(rc);
        rc = ble_hs_id_infer_auto(0, &g_own_addr_type);
        LOG(rc, g_own_addr_type);
        int len = 6;
        tos::mac_addr_t mac;

        ble_hs_id_copy_addr(g_own_addr_type, mac.addr.data(), &len);
        LOG(tos::span<const uint8_t>(mac.addr));
        sync = true;

        ble_gatts_start();

        adv_sem.up_isr();
    };

    ble_hs_cfg.reset_cb = [](auto x) {
        sync = false;
        LOG("Reset callback", x);
    };

    auto rc = ble_svc_gap_device_name_set(device_name);
    LOG(rc);

    tos::launch(tos::alloc_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            LOG("Tick");
            tos::this_thread::sleep_for(*alarm, 10s);
        }
    });

    while (true) {
        using namespace std::chrono_literals;
        adv_sem.down();
        if (!sync) {
            continue;
        }
        // alarm->sleep_for(10s);
        // LOG("Wake",
        //     systicks.get(),
        //     alarm->m_base_alarm.m_sleepers.front().sleep_ticks,
        //     alarm->m_base_alarm.m_sleepers.back().sleep_ticks,
        //     alarm->m_base_alarm.m_timer->get_counter(),
        //     alarm->m_base_alarm.m_timer->get_period(),
        //     alarm->m_base_alarm.running,
        //     alarm->m_base_alarm.m_timer->running);
        advertise();
    }
    tos::this_thread::block_forever();
}

tos::stack_storage<> stak;
void tos_main() {
    tos::launch(stak, entry);
}