//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>
#include <tos/compiler.hpp>

#include <arch/drivers.hpp>
#include <algorithm>
#include <tos/print.hpp>
#include <tos/fixed_fifo.hpp>
#include <common/alarm.hpp>
#include <common/lcd.hpp>
#include <tos/expected.hpp>
#include <string_view>
#include <ble_services/ble_nus/ble_nus.h>
#include <ble.h>

#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "ble_nus.h"
#include "app_timer.h"

static uint32_t m_nus_link_ctx_storage_ctx_data_pool[((20)) * (((sizeof(ble_nus_client_context_t)) + 3) >> 2)];
static blcm_link_ctx_storage_t m_nus_link_ctx_storage = {m_nus_link_ctx_storage_ctx_data_pool, 20,
                                                         sizeof(m_nus_link_ctx_storage_ctx_data_pool) / 20};
static ble_nus_t m_nus = []{
    ble_nus_t res {
        0,
        0,
        {},
        {},
        &m_nus_link_ctx_storage,
        {}
    };
    return res;
}();

static nrf_sdh_ble_evt_observer_t m_nus_obs __attribute__((section(".sdh_ble_observers2"))) __attribute__((used)) = {
    ble_nus_on_ble_evt,
    &m_nus
};
static nrf_ble_gatt_t m_gatt;
static nrf_sdh_ble_evt_observer_t m_gatt_obs __attribute__((section(".sdh_ble_observers1")))__attribute__((used)) = {nrf_ble_gatt_on_ble_evt, &m_gatt};
static nrf_ble_qwr_t m_qwr;
static nrf_sdh_ble_evt_observer_t m_qwr_obs __attribute__((section(".sdh_ble_observers2")))__attribute__((used)) = {nrf_ble_qwr_on_ble_evt, &m_qwr};                                                             /**< Context for the Queued Write module.*/
static ble_advertising_t m_advertising;

static nrf_sdh_ble_evt_observer_t m_advertising_ble_obs __attribute__((section(".sdh_ble_observers1")))__attribute__((used)) = {ble_advertising_on_ble_evt, &m_advertising};

static nrf_sdh_soc_evt_observer_t m_advertising_soc_obs __attribute__((section(".sdh_soc_observers1")))__attribute__((used)) = {ble_advertising_on_sys_evt, &m_advertising};                                                 /**< Advertising module instance. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
    {
        {0xDDDD, BLE_UUID_TYPE_VENDOR_BEGIN}
    };

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGI N                 /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                1800//0                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(40, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(150, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

tos::fixed_fifo<char, 1024> buf;

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
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

        case BLE_GAP_EVT_TIMEOUT:
        {
            buf.push('\n');
            buf.push('T');
            buf.push('O');
            buf.push('\n');
            break;
        }

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
            // No implementation needed.
            break;
    }
}

static nrf_sdh_ble_evt_observer_t m_ble_observer __attribute__((section(".sdh_ble_observers3")))__attribute__((used)) = {
    ble_evt_handler, nullptr};

tos::function_ref<void(const ble_evt_t&)> obs{[](ble_evt_t const&, void*) {}};

static nrf_sdh_ble_evt_observer_t tos_ble_observer
    __attribute__((section(".sdh_ble_observers1"))) __attribute__((used))
    { [](ble_evt_t const* ev, void*) {
        obs(*ev);
    }, nullptr };

auto handler = [](ble_evt_t const & ev){
    if (ev.header.evt_id == BLE_GAP_EVT_CONNECTED)
    {
        buf.push('C');
        buf.push('\n');
    }
};

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
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
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            //APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            buf.push('\n');
            buf.push('T');
            buf.push('O');
            buf.push('O');
            buf.push('\n');
            break;
        default:
            break;
    }
}

static void advertising_init()
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

static void advertising_start()
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

namespace tos
{
namespace nrf52
{
enum class setname_errors
{
    invalid_addr = NRF_ERROR_INVALID_ADDR,
    invalid_param = NRF_ERROR_INVALID_PARAM,
    bad_size = NRF_ERROR_DATA_SIZE,
    forbidden = NRF_ERROR_FORBIDDEN
};

class softdev
{
public:
    softdev()
    {
        ret_code_t err_code;

        err_code = nrf_sdh_enable_request();
        //APP_ERROR_CHECK(err_code);
        if (err_code != NRF_SUCCESS)
        {
            tos::this_thread::block_forever();
        }

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        uint32_t ram_start = 0;
        err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
        APP_ERROR_CHECK(err_code);

        // Enable BLE stack.
        err_code = nrf_sdh_ble_enable(&ram_start);
        APP_ERROR_CHECK(err_code);
    }

    expected<void, setname_errors>
    set_device_name(std::string_view name)
    {
        ble_gap_conn_sec_mode_t sec_mode;

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

        auto err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *) name.data(),
                                              name.size());

        if (err_code != NRF_SUCCESS)
        {
            return unexpected(setname_errors(err_code));
        }

        return {};
    }

    void set_conn_params()
    {

    }

private:
};
}
}

static void gap_params_init(tos::nrf52::softdev& sd)
{
    auto r = sd.set_device_name("Tos BLE");

    if (!r) return;

    ble_gap_conn_params_t gap_conn_params {};

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    auto err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

auto g = tos::open(tos::devs::gpio);
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        auto str = tos::itoa(p_evt->params.rx_data.length, 10);
        while (*str)
        {
            buf.push(*str++);
        }
        buf.push('\n');
        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            g->write(14, tos::digital::low);
            buf.push(p_evt->params.rx_data.p_data[i]);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            buf.push('\n');
        }
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init;

    // Initialize Queued Write Module.
    qwr_init.error_handler = [](uint32_t nrf_error) {
        APP_ERROR_HANDLER(nrf_error);
    };

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init()
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init{};

    cp_init.p_conn_params                  = nullptr;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

auto ble_task = [](auto& g)
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    g->write(13, digital::low);

    tos::println(usart, "hello");

    tos::nrf52::softdev sd;
    tos::println(usart, "sd initd");
    gap_params_init(sd);
    tos::println(usart, "gap initd");
    gatt_init();
    tos::println(usart, "gatt initd");
    services_init();
    tos::println(usart, "services initd");
    advertising_init();
    tos::println(usart, "adv initd");
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    conn_params_init();
    tos::println(usart, "conn initd");
    advertising_start();
    tos::println(usart, "began adv");

    while (true)
    {
        auto c = buf.pop();
        usart->write({&c, 1});
    }

    tos::this_thread::block_forever();
};

void TOS_EXPORT tos_main()
{
    using namespace tos;
    g->set_pin_mode(13, pin_mode::out);
    g->set_pin_mode(14, pin_mode::out);

    tos::launch(tos::alloc_stack, ble_task, g);
    //tos::launch(i2c_task);
    //tos::launch(tos::alloc_stack, led2_task);
}