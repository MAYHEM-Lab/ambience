#pragma once

#include <stdint.h>
#include <tos/mutex.hpp>
#include <arch/drivers.hpp>

typedef enum {
  SPBTLERF_OK = 0,
  SPBTLERF_ERROR
} SPBTLERF_state_t;

typedef enum {
  DISABLE_LOW_POWER_MODE = 0,
  ENABLE_LOW_POWER_MODE
} lowPowerMode_t;

void attach_HCI_CB(void (*callback)(void *pckt));

namespace tos::stm32{
    struct spi;
}

extern bool isr_enabled;
class spbtle_rf : public tos::tracked_driver<spbtle_rf, 1>
{
    // needs timer + spi
  public:
    using SpiT = tos::stm32::spi*;
    using GpioT = tos::stm32::gpio;
    using PinT = GpioT::pin_type;

    spbtle_rf(SpiT SPIx, PinT csPin, PinT spiIRQ, PinT reset);

    SPBTLERF_state_t begin();

    void update();

    bool data_present() const {
        return m_gpio.read(m_irq_pin);
    }

    void reset()
    {
        using namespace std::chrono_literals;
        m_gpio.write(m_reset, tos::digital::low);
        alarm_ptr->sleep_for(5ms);
        m_gpio.write(m_reset, tos::digital::high);
        alarm_ptr->sleep_for(5ms);
    }

    int spi_write(tos::span<const uint8_t> d1,
                    tos::span<const uint8_t> d2);

    int spi_read(tos::span<uint8_t> buf);

    void bootloader();

    ~spbtle_rf() {
        m_gpio.write(m_reset, tos::digital::low);
        isr_enabled = false;
    }

private:

    tos::any_alarm* alarm_ptr;
    GpioT m_gpio;
    SpiT m_spi;
    PinT m_cs, m_irq_pin, m_reset;
    tos::mutex m_spi_prot;
};

#include "spbtle_rf.inl"

