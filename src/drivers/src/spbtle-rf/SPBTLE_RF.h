#pragma once

#include <cstdint>
#include <tos/mutex.hpp>
#include <arch/drivers.hpp>
#include <tos/expected.hpp>
#include <common/ble/address.hpp>

enum class spbtle_errors {
    unknown,
    timeout = 0xFF
};

namespace tos {
    namespace spbtle {
        struct fw_id {
            uint16_t build_number;
        };

        class gatt {

        };

        struct advertising {
        public:
            advertising(std::chrono::milliseconds interval, std::string_view local_name);

            void stop();
        private:
        };

        class gap {
        public:
            gap(gatt &, std::string_view name);

            /**
             * Sets the device's GAP name
             *
             * Length of the new name must be less than or equal to the
             * length of the original name.
             *
             * @param name name of the device
             */
            void set_device_name(std::string_view name);

            advertising
            initialize_advertising();

            ~gap();

        private:
            uint16_t service_handle;
            uint16_t dev_name_char_handle;
            uint16_t appearance_char_handle;

            uint8_t m_name_len;
        };
    }
}

namespace tos::stm32 {
    struct spi;
}

extern bool isr_enabled;

class spbtle_rf : public tos::tracked_driver<spbtle_rf, 1> {
public:
    using SpiT = tos::stm32::spi *;
    using GpioT = tos::stm32::gpio;
    using PinT = GpioT::pin_type;
    using ExtiT = tos::stm32::exti *;

    spbtle_rf(
            SpiT spi,
            ExtiT exti,
            tos::any_alarm &alarm,
            PinT cs,
            PinT irq_pin,
            PinT reset);

    tos::expected<tos::spbtle::gatt, spbtle_errors>
    initialize_gatt();

    tos::spbtle::gap
    initialize_gap(tos::spbtle::gatt &, std::string_view name);

    bool data_present() const {
        return m_gpio->read(m_irq_pin);
    }

    tos::expected<tos::spbtle::fw_id, spbtle_errors>
    get_fw_id() const;

    void reset() {
        using namespace std::chrono_literals;
        m_gpio->write(m_reset, tos::digital::low);
        m_alarm_ptr->sleep_for(5ms);
        m_gpio->write(m_reset, tos::digital::high);
        m_alarm_ptr->sleep_for(5ms);
    }

    int spi_write(tos::span<const uint8_t> d1,
                  tos::span<const uint8_t> d2);

    int spi_read(tos::span<uint8_t> buf);

    void bootloader();

    ~spbtle_rf() {
        m_exti->detach(m_irq_pin);
        m_gpio->write(m_reset, tos::digital::low);
        isr_enabled = false;
    }

private:

    tos::expected<void, spbtle_errors> begin();

    tos::any_alarm *m_alarm_ptr;
    GpioT m_gpio;
    SpiT m_spi;
    ExtiT m_exti;
    PinT m_cs, m_irq_pin, m_reset;
    tos::mutex m_spi_prot;
};

extern void attach_HCI_CB(tos::function_ref<void(void*)> callback);

#include "spbtle_rf.inl"
