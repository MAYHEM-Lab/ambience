extern "C" {
#include "bluenrg_aci.h"
#include "bluenrg_gap.h"
#include <hci.h>
}

#include <bluenrg_interface.hpp>
#include <common/ble/address.hpp>
#include <common/spi.hpp>
#include <tos/device/spbtlerf/adapter.hpp>

namespace tos::device::spbtle {
namespace {
class interface_impl : public bluenrg_interface {
public:
    int32_t read(uint8_t* buffer, uint8_t buff_size) override {
        return adapter::instance()->cb_spi_read({buffer, buff_size});
    }

    int32_t BlueNRG_SPI_Write(uint8_t* data1,
                              uint8_t* data2,
                              uint8_t Nb_bytes1,
                              uint8_t Nb_bytes2) {
        return adapter::instance()->cb_spi_write({data1, Nb_bytes1},
                                                 {data2, Nb_bytes2});
    }

    void write(const void* data1,
                          const void* data2,
                          int32_t n_bytes1,
                          int32_t n_bytes2) override {
        while (true) {
            auto res =
                BlueNRG_SPI_Write((uint8_t*)data1, (uint8_t*)data2, n_bytes1, n_bytes2);
            if (res == 0) {
                break;
            }
            tos::this_thread::yield();
        }
    }

    void disable_irq() override {
        tos::device::spbtle::adapter::instance()->cb_spi_disable_irq();
    }

    void enable_irq() override {
        tos::device::spbtle::adapter::instance()->cb_spi_enable_irq();
    }

    uint8_t data_present() override {
        return adapter::instance()->cb_data_present();
    }

    tos::any_alarm* get_alarm() override {
        return adapter::instance()->cb_alarm();
    }

    void event_cb(void* pckt) override {
        adapter::instance()->cb_hci_event_call(pckt);
    }
};
interface_impl _if;
}
}

namespace tos::device::spbtle {
void adapter::cb_reset() {
    using namespace std::chrono_literals;
    m_config.gpio->write(m_config.reset_pin, tos::digital::low);
    tos::this_thread::sleep_for(m_config.alarm, 5ms);
    m_config.gpio->write(m_config.reset_pin, tos::digital::high);
    tos::this_thread::sleep_for(m_config.alarm, 5ms);
}

bool adapter::cb_data_present() const {
    return m_config.gpio->read(m_config.m_irq_pin);
}

void adapter::cb_bootloader() {
    cb_spi_disable_irq();

    m_config.gpio->set_pin_mode(m_config.m_irq_pin, tos::pin_mode::out);
    m_config.gpio->write(m_config.m_irq_pin, tos::digital::high);

    cb_reset();
    cb_spi_enable_irq();
}

void adapter::cb_hci_event_call(void* packet) {
    m_event_handler(packet);
}

void adapter::cb_spi_enable_irq() {
    m_irq_enabled = true;
}

void adapter::cb_spi_disable_irq() {
    m_irq_enabled = false;
}

int adapter::cb_spi_write(tos::span<const uint8_t> d1, tos::span<const uint8_t> d2) {
    int32_t result = 0;

    tos::lock_guard lock(m_spi_mutex);
    cb_spi_disable_irq();

    tos::pull_low_guard cs_guard{*m_config.gpio, m_config.cs_pin};
    unsigned char header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};

    // | Ready | WBUF | 0 | RBUF | 0 |
    unsigned char header_slave[5] = {0xaa, 0x00, 0x00, 0x00, 0x00};

    /*for (auto& [s, h] :
            ranges::zip_view(header_slave, header_master))
    {
        s = m_spi->exchange8(h);
    }*/

    auto header_res = m_config.spi->exchange(header_slave, header_master);
    if (!header_res) {
        return -3;
    }

    if (header_slave[0] == 0x02) {
        /* SPI is ready */
        if (header_slave[1] >= (d1.size() + d2.size())) {
            if (!d1.empty()) {
                auto r1 = m_config.spi->write(d1);
                if (!r1) {
                    result = -4;
                }
            }
            if (!d2.empty()) {
                auto r1 = m_config.spi->write(d2);
                if (!r1) {
                    result = -5;
                }
            }
        } else {
            /* Buffer is too small */
            result = -2;
        }
    } else {
        /* SPI is not ready */
        result = -1;
    }

    cb_spi_enable_irq();

    return result;
}
int adapter::cb_spi_read(tos::span<uint8_t> buf) {
    tos::lock_guard lock(m_spi_mutex);

    tos::pull_low_guard cs_guard{*m_config.gpio, m_config.cs_pin};

    uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};
    uint8_t header_slave[5];

    auto header_res = m_config.spi->exchange(header_slave, header_master);
    if (!header_res) {
        return -1;
    }

    uint16_t byte_count = 0;
    if (header_slave[0] == 0x02) {
        /* device is ready */
        byte_count = (uint16_t(header_slave[4]) << 8) | header_slave[3];

        if (byte_count > 0) {
            byte_count = std::min<uint16_t>(byte_count, buf.size());

            for (uint16_t len = 0; len < byte_count; len++) {
                auto res = tos::spi::exchange(m_config.spi, 0xff);
                if (!res) {
                    return -2;
                }
                buf[len] = force_get(res);
            }
        }
    }

    // Add a small delay to give time to the BlueNRG to set the IRQ pin low
    // to avoid a useless SPI read at the end of the transaction
    for (int i = 0; i < 2; i++) asm("nop");

    return byte_count;
}

void adapter::exti_handler() {
    m_spi_sem.up_isr();
}

void adapter::spi_thread() {
    while (true) {
        m_spi_sem.down();
        if (!m_irq_enabled) {
            m_spi_sem.up();
            tos::this_thread::yield();
            continue;
        }
        HCI_Isr();
        HCI_Process();
    }
}

adapter::adapter(const adapter_config& conf)
    : m_config(conf) {
    begin();
}

expected<intrusive_ptr<adapter>, errors> adapter::open(const adapter_config& config) {
    m_instance = make_intrusive<adapter>(config);
    return m_instance;
}

adapter* adapter::instance() {
    return m_instance.get();
}

void adapter::begin() {
    namespace digital = tos::digital;

    m_thread = &tos::launch(tos::alloc_stack, [this] { spi_thread(); });

    m_config.gpio->set_pin_mode(m_config.cs_pin, tos::pin_mode::out);
    m_config.gpio->write(m_config.cs_pin, digital::high);

    m_config.gpio->set_pin_mode(m_config.m_irq_pin, tos::pin_mode::in_pulldown);

    m_config.exti->attach(m_config.m_irq_pin,
                          tos::pin_change::rising,
                          mem_function_ref<&adapter::exti_handler>(*this));

    // Enable SPI EXTI interrupt
    cb_spi_enable_irq();

    // Configure Reset pin
    m_config.gpio->set_pin_mode(m_config.reset_pin, tos::pin_mode::out);
    m_config.gpio->write(m_config.reset_pin, digital::high);

    /* Initialize the BlueNRG HCI */
    HCI_Init(_if);

    cb_reset();
}

void adapter::power_off() {
    m_config.gpio->write(m_config.reset_pin, digital::low);
}

adapter::~adapter() {
    m_config.exti->detach(m_config.m_irq_pin);
    power_off();
    m_irq_enabled = false;
}

tos::expected<void, errors> adapter::set_public_address(const ble::address_t& address) {
    auto ret = aci_hal_write_config_data(
        CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, address.addr);
    if (ret != BLE_STATUS_SUCCESS) {
        return tos::unexpected(errors::unknown);
    }

    return {};
}

tos::expected<ble::address_t, errors> adapter::get_mac_address() const {
    return tos::unexpected(errors::unknown);
}

tos::expected<fw_id, errors> adapter::get_fw_id() const {
    fw_id build_number{};
    auto res = aci_hal_get_fw_build_number(&build_number.build_number);
    if (res) {
        return tos::unexpected(static_cast<errors>(res));
    }
    return build_number;
}

tos::expected<gatt, errors> adapter::initialize_gatt() {
    auto res = aci_gatt_init();
    if (res) {
        return tos::unexpected(errors::unknown);
    }
    return gatt{};
}

gap adapter::initialize_gap(gatt& g, std::string_view name) {
    return gap(g, name);
}

void gap::set_device_name(std::string_view name) {
    auto ret = aci_gatt_update_char_value(
        service_handle, dev_name_char_handle, 0, name.size(), (uint8_t*)name.data());
    if (ret) {
        tos::debug::panic("Can't set name");
    }
    m_name_len = name.size();
}

gap::gap(gatt&, std::string_view name) {
    auto ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1,
                                    false,
                                    name.size(),
                                    &service_handle,
                                    &dev_name_char_handle,
                                    &appearance_char_handle);
    if (ret) {
        tos::debug::panic("Can't init gap");
    }
    set_device_name(name);
}

gap::~gap() {
}


expected<void, errors> advertising::start(std::chrono::milliseconds interval,
                                          std::string_view local_name) {
    auto name = char(AD_TYPE_COMPLETE_LOCAL_NAME) + std::string(local_name);
    auto ret = aci_gap_set_discoverable(ADV_IND,
                                        (interval.count() * 1000) / 625,
                                        (interval.count() * 1000) / 625,
                                        PUBLIC_ADDR,
                                        NO_WHITE_LIST_USE,
                                        name.size(),
                                        name.data(),
                                        0,
                                        nullptr,
                                        0xFFFF,
                                        0xFFFF);
    if (ret != BLE_STATUS_SUCCESS) {
        return unexpected(static_cast<errors>(ret));
    }
    return {};
}

void advertising::stop() {
    aci_gap_set_non_discoverable();
}

void adapter::service_created(gatt_service& serv) {
    m_event_handler.register_service(serv);
}

void adapter::service_destroyed(gatt_service& serv) {
    m_event_handler.remove_service(serv);
}
} // namespace tos::device::spbtle