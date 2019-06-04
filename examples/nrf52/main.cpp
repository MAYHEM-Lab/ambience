//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <arch/drivers.hpp>

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/track_ptr.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>
#include <tos/compiler.hpp>

#include <algorithm>
#include <tos/print.hpp>
#include <tos/fixed_fifo.hpp>
#include <common/alarm.hpp>
#include <common/lcd.hpp>
#include <tos/expected.hpp>
#include <string_view>
#include <common/epd/waveshare/bw29.hpp>

#include <ble.h>
#include <common/ble_srv_common.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrf_ble_gatt.h"
#include "bakir_ble.hpp"

#include "text.hpp"
#include "canvas.hpp"

static nrf_ble_gatt_t m_gatt;
static nrf_sdh_ble_evt_observer_t m_gatt_obs __attribute__((section(".sdh_ble_observers1")))__attribute__((used)) = {nrf_ble_gatt_on_ble_evt, &m_gatt};

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void *)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            ble_gap_phys_t const phys =
            {
                BLE_GAP_PHY_AUTO,
                BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, nullptr, nullptr);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, nullptr, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

static nrf_sdh_ble_evt_observer_t m_ble_observer __attribute__((section(".sdh_ble_observers3")))__attribute__((used)) = {
    ble_evt_handler, nullptr};

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t *, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
    }
}

/**@brief Function for initializing the GATT library. */
void gatt_init()
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init()
{
    ble_gap_conn_params_t gap_conn_params {};

    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(40, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(150, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = 0;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS);

    auto err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

bakir::canvas<128, 296> framebuf;
constexpr auto font =
    bakir::basic_font()
        .inverted()             // black on white
        .mirror_horizontal()    // left to right
        .rotate_90_cw();        // screen is rotated

auto ble_task = []()
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    auto g = open(tos::devs::gpio);
    auto usart = open(tos::devs::usart<0>, usconf);

    auto mosi = 33_pin;
    auto clk = 34_pin;
    auto cs = 35_pin;
    auto dc = 36_pin;
    auto reset = 37_pin;
    auto busy = 38_pin;

    g->set_pin_mode(cs, tos::pin_mode::out);
    g->write(cs, tos::digital::high);

    tos::println(usart, "hello");

    tos::nrf52::softdev sd;
    auto setname_res = sd.set_device_name("Tos BLE");
    auto tx_pow_res = sd.set_tx_power();
    tos::println(usart, "sd initd", bool(setname_res));
    gap_params_init();
    tos::println(usart, "gap initd");
    gatt_init();
    tos::println(usart, "gatt initd");
    auto serv = bakir::make_ble_service(usart);
    tos::println(usart, "services initd");
    tos::nrf52::advertising adv;
    tos::println(usart, "adv initd");

    auto started = adv.start();
    if (started)
    {
        tos::println(usart, "began adv");
    }
    else
    {
        tos::println(usart, "adv failed");
    }

    {
        framebuf.fill(true);
        auto print_str = [&](auto& str, size_t x, size_t y) {
            for (char c : str) {
                if (c == 0) return;
                auto ch = font.get(c);
                if (!ch)
                {
                    ch = font.get('?');
                }
                framebuf.copy(*ch, x, y);
                y += ch->height();
            }
        };
        print_str("Hello world", 96, 24);

        tos::nrf52::spi s(clk, 39_pin, mosi);
        epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
        epd.initialize([](std::chrono::milliseconds ms) {
            nrf_delay_ms(ms.count());
        });
        epd.SetFrameMemory(framebuf.data(), 0, 0, epd.width, epd.height);
        epd.DisplayFrame();
    }

    while (true)
    {
        std::array<char, 1> c;
        serv->read(c);

        if (c[0] == 'a')
        {
            tos::nrf52::spi s(clk, 39_pin, mosi);

            epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
            epd.initialize([](std::chrono::milliseconds ms) {
                nrf_delay_ms(ms.count());
            });

            epd.SetFrameMemory(framebuf.data(), 0, 0, epd.width, epd.height);
            epd.DisplayFrame();
        }
        else if (c[0] == 'c')
        {
            tos::nrf52::spi s(clk, 39_pin, mosi);

            epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
            epd.initialize([](std::chrono::milliseconds ms) {
                nrf_delay_ms(ms.count());
            });

            epd.ClearFrameMemory(0xFF);
            epd.DisplayFrame();
        }

        usart->write(c);
    }

    tos::this_thread::block_forever();
};

void TOS_EXPORT tos_main()
{
    tos::launch(tos::alloc_stack, ble_task);
}