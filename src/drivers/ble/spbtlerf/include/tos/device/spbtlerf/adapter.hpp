#pragma once

#include <common/alarm.hpp>
#include <cstdint>
#include <tos/device/spbtlerf/common.hpp>
#include <tos/expected.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/span.hpp>
#include <arch/drivers.hpp>
#include <tos/mutex.hpp>
#include <tos/device/spbtlerf/events.hpp>
#include <common/ble/address.hpp>

namespace tos::device::spbtle {
struct adapter_config {
    any_alarm* alarm;
    stm32::exti* exti;
    stm32::gpio* gpio;
    stm32::spi* spi;
    stm32::pin_t cs_pin, m_irq_pin, reset_pin;
};


class gatt {};

struct advertising {
public:
    advertising() = default;

    expected<void, errors> start(std::chrono::milliseconds interval,
                                 std::string_view local_name);

    void stop();

private:
};

class gap {
public:
    gap(gatt&, std::string_view name);

    /**
     * Sets the device's GAP name
     *
     * Length of the new name must be less than or equal to the
     * length of the original name.
     *
     * @param name name of the device
     */
    void set_device_name(std::string_view name);

    advertising initialize_advertising();

    ~gap();

private:
    uint16_t service_handle;
    uint16_t dev_name_char_handle;
    uint16_t appearance_char_handle;

    uint8_t m_name_len;
};

class adapter : public ref_counted<adapter> {
public:
    adapter(const adapter_config& conf);

    static expected<intrusive_ptr<adapter>, errors> open(const adapter_config& config);

    tos::expected<gatt, errors> initialize_gatt();

    gap initialize_gap(gatt&, std::string_view name);

    tos::expected<fw_id, errors> get_fw_id() const;
    tos::expected<ble::address_t, errors> get_mac_address() const;

    tos::expected<void, errors> set_public_address(const ble::address_t& address);

    /**
     * Due to the design of the ST provided driver, we can have only 1 instance of the
     * driver.
     */
    static adapter* instance();

    void cb_reset();
    bool cb_data_present() const;
    void cb_bootloader();
    void cb_hci_event_call(void* packet);

    int cb_spi_write(tos::span<const uint8_t> d1, tos::span<const uint8_t> d2);
    int cb_spi_read(tos::span<uint8_t> buf);
    void cb_spi_enable_irq();
    void cb_spi_disable_irq();

    void service_created(gatt_service& serv);
    void service_destroyed(gatt_service& serv);

    tos::any_alarm* cb_alarm() {
        return m_config.alarm;
    }

    ~adapter();

private:

    void begin();

    void exti_handler();
    void spi_thread();

    tos::kern::tcb* m_thread;
    semaphore m_spi_sem{0};
    hci_evt_handler m_event_handler;
    adapter_config m_config;
    mutex m_spi_mutex;
    bool m_irq_enabled = false;
    static inline intrusive_ptr<adapter> m_instance;
};
} // namespace tos::device::spbtle