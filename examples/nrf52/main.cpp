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
            using namespace tos;
            g->write(14_pin, tos::digital::low);
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


#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE 0xFF

static const uint8_t LUTDefault_full[] =
    {
        WRITE_LUT_REGISTER,  // command
        0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
        0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
        0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
        0x35, 0x51, 0x51, 0x19, 0x01, 0x00
    };

static const uint8_t LUTDefault_part[] =
    {
        WRITE_LUT_REGISTER,  // command
        0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

template <class SpiT>
class epd {
public:
    using PinT = typename std::remove_pointer_t<SpiT>::gpio_type::pin_type;
    static const uint16_t WIDTH = 128;
    static const uint16_t HEIGHT = 296;

    uint16_t width = 128;
    uint16_t height = 296;

    explicit epd(SpiT spi, PinT cs, PinT dc, PinT reset, PinT busy)
        : m_spi{std::move(spi)}
        , m_cs{cs}
        , m_dc{dc}
        , m_reset{reset}
        , m_busy{busy}
    {
        m_g.set_pin_mode(m_dc, tos::pin_mode::out);
        m_g.write(m_dc, tos::digital::high);

        m_g.set_pin_mode(m_reset, tos::pin_mode::out);
        m_g.write(m_reset, tos::digital::high);

        m_g.set_pin_mode(m_busy, tos::pin_mode::in);
    }

    void _reset()
    {
        using namespace std::chrono_literals;
        m_g.write(m_reset, tos::digital::low);
        nrf_delay_ms(200);
        //tos::delay_ms(20ms);
        m_g.write(m_reset, tos::digital::high);
        nrf_delay_ms(200);
        //tos::delay_ms(20ms);
    }

    void Init()
    {
        _reset();
        _writeCommand(DRIVER_OUTPUT_CONTROL); // Panel configuration, Gate selection
        _writeData((HEIGHT - 1) % 256);
        _writeData((HEIGHT - 1) / 256);
        _writeData(0x00);
        _writeCommand(BOOSTER_SOFT_START_CONTROL); // softstart
        _writeData(0xd7);
        _writeData(0xd6);
        _writeData(0x9d);
        _writeCommand(WRITE_VCOM_REGISTER); // VCOM setting
        _writeData(0xa8);    // * different
        _writeCommand(SET_DUMMY_LINE_PERIOD); // DummyLine
        _writeData(0x1a);    // 4 dummy line per gate
        _writeCommand(SET_GATE_TIME); // Gatetime
        _writeData(0x08);    // 2us per line
        _writeCommand(BORDER_WAVEFORM_CONTROL);
        _writeData(0x03);
        _writeCommand(DATA_ENTRY_MODE_SETTING);
        _writeData(0x03); // X increment; Y increment
        _writeCommandDataPGM(LUTDefault_full, sizeof(LUTDefault_full));
        //_setPartialRamArea(0, 0, WIDTH, HEIGHT);
    }

    void SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
        SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
        /* x point must be the multiple of 8 or the last 3 bits will be ignored */
        SendData((x_start >> 3) & 0xFF);
        SendData((x_end >> 3) & 0xFF);
        SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
        SendData(y_start & 0xFF);
        SendData((y_start >> 8) & 0xFF);
        SendData(y_end & 0xFF);
        SendData((y_end >> 8) & 0xFF);
    }

    void SetMemoryPointer(int x, int y) {
        SendCommand(SET_RAM_X_ADDRESS_COUNTER);
        /* x point must be the multiple of 8 or the last 3 bits will be ignored */
        SendData((x >> 3) & 0xFF);
        SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
        SendData(y & 0xFF);
        SendData((y >> 8) & 0xFF);
        WaitUntilIdle();
    }

    void ClearFrameMemory(unsigned char color) {
        SetMemoryArea(0, 0, this->width - 1, this->height - 1);
        SetMemoryPointer(0, 0);
        SendCommand(WRITE_RAM);
        /* send the color data */
        for (int i = 0; i < this->width / 8 * this->height; i++) {
            SendData(color);
        }
    }

    void DisplayFrame(void) {
        SendCommand(DISPLAY_UPDATE_CONTROL_2);
        SendData(0xC4);
        SendCommand(MASTER_ACTIVATION);
        SendCommand(TERMINATE_FRAME_READ_WRITE);
        WaitUntilIdle();
    }

    void WaitUntilIdle()
    {
        // active low
        while (m_g.read(m_busy)) {
            tos::this_thread::yield();
        }
    }

private:

    void SendCommand(unsigned char command) {
        _writeCommand(command);
    }

    void SendData(unsigned char data) {
        _writeData(data);
    }

    void _writeCommand(uint8_t c)
    {
        m_g.write(m_dc, tos::digital::low);
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(c);
        m_g.write(m_cs, tos::digital::high);
        m_g.write(m_dc, tos::digital::high);
    }

    void _writeData(uint8_t d)
    {
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(d);
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeData(const uint8_t* data, uint16_t n) {
        m_g.write(m_cs, tos::digital::low);
        m_spi->write({data, n});
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
    {
        m_g.write(m_dc, tos::digital::low);
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(*pCommandData++);
        datalen--;
        m_g.write(m_dc, tos::digital::high);
        m_spi->write({pCommandData, datalen});
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeCommandDataPGM(const uint8_t* pCommandData, uint8_t datalen)
    {
        std::vector<uint8_t> buf(pCommandData, pCommandData + datalen);
        _writeCommandData(buf.data(), buf.size());
    }

    void _writeDataPGM(const uint8_t* data, uint16_t n)
    {
        std::vector<uint8_t> buf(data, data + n);
        _writeData(buf.data(), buf.size());
    }

    SpiT m_spi;
    tos::nrf52::gpio m_g;
    tos::nrf52::gpio::pin_type m_cs, m_dc, m_reset, m_busy;
};

auto ble_task = [](auto& g)
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto mosi = 33_pin;
    auto clk = 34_pin;
    auto cs = 35_pin;
    auto dc = 36_pin;
    auto reset = 37_pin;
    auto busy = 38_pin;

    g->set_pin_mode(cs, tos::pin_mode::out);
    g->write(cs, tos::digital::high);

    tos::nrf52::spi s(clk, 39_pin, mosi);

    epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
    epd.Init();

    epd.ClearFrameMemory(0x00);
    epd.DisplayFrame();
    epd.ClearFrameMemory(0x00);
    epd.DisplayFrame();

    epd.ClearFrameMemory(0xFF);
    epd.DisplayFrame();
    epd.ClearFrameMemory(0xFF);
    epd.DisplayFrame();

    g->write(13_pin, digital::low);

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
    g->set_pin_mode(13_pin, pin_mode::out);
    g->set_pin_mode(14_pin, pin_mode::out);

    tos::launch(tos::alloc_stack, ble_task, g);
    //tos::launch(i2c_task);
    //tos::launch(tos::alloc_stack, led2_task);
}