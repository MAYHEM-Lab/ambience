#include "hci.h"
#include "stm32_bluenrg_ble.h"

#include <SPBTLE_RF.h>
#include <tos/mutex.hpp>

bool isr_enabled = false;
tos::any_alarm* alarm_ptr;

namespace {
tos::function_ref<void(void*)> HCI_callback{[](void*, void*) {}};

tos::semaphore isr_sem{0};

void exti_handler(void*) {
    isr_sem.up_isr();
}

void ble_isr() {
    while (true) {
        isr_sem.down();
        if (!isr_enabled) {
            isr_sem.up();
            tos::this_thread::yield();
            continue;
        }
        HCI_Isr();
        HCI_Process();
    }
}
} // namespace

spbtle_rf::spbtle_rf(
    SpiT SPIx, ExtiT exti, tos::any_alarm& alarm, PinT cs, PinT spiIRQ, PinT reset)
    : tracked_driver(0)
    , m_alarm_ptr{&alarm}
    , m_spi{std::move(SPIx)}
    , m_exti{exti}
    , m_cs{cs}
    , m_irq_pin{spiIRQ}
    , m_reset{reset} {
    ::alarm_ptr = &alarm;
    begin();
}

static tos::stack_storage<2048> sstorage;

tos::expected<void, spbtle_errors> spbtle_rf::begin() {
    namespace digital = tos::digital;

    tos::launch(sstorage, ble_isr);

    m_gpio->set_pin_mode(m_cs, tos::pin_mode::out);
    m_gpio->write(m_cs, digital::high);

    m_gpio->set_pin_mode(m_irq_pin, tos::pin_mode::in_pulldown);

    m_exti->attach(
        m_irq_pin, tos::pin_change::rising, tos::function_ref<void()>(&exti_handler));

    // Enable SPI EXTI interrupt
    isr_enabled = true;

    // Configure Reset pin
    m_gpio->set_pin_mode(m_reset, tos::pin_mode::out);
    m_gpio->write(m_reset, digital::low);

    /* Initialize the BlueNRG HCI */
    HCI_Init();

    reset();

    return {};
}

extern "C" void Hal_Write_Serial(const void* data1,
                                 const void* data2,
                                 int32_t n_bytes1,
                                 int32_t n_bytes2) {
    while (true) {
        auto res =
            BlueNRG_SPI_Write((uint8_t*)data1, (uint8_t*)data2, n_bytes1, n_bytes2);
        if (res == 0) {
            break;
        }
        tos::this_thread::yield();
    }
}

void BlueNRG_RST(void) {
    spbtle_rf::get(0)->reset();
}

uint8_t BlueNRG_DataPresent(void) {
    return spbtle_rf::get(0)->data_present();
}

void BlueNRG_HW_Bootloader(void) {
    spbtle_rf::get(0)->bootloader();
}

int32_t BlueNRG_SPI_Read_All(uint8_t* buffer, uint8_t buff_size) {
    return spbtle_rf::get(0)->spi_read({buffer, buff_size});
}

int32_t
BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2) {
    return spbtle_rf::get(0)->spi_write({data1, Nb_bytes1}, {data2, Nb_bytes2});
}

void Enable_SPI_IRQ(void) {
    isr_enabled = true;
}

void Disable_SPI_IRQ(void) {
    isr_enabled = false;
}

void attach_HCI_CB(tos::function_ref<void(void*)> callback) {
    HCI_callback = callback;
}

void HCI_Event_CB(void* pckt) {
    HCI_callback(pckt);
}
