/**
 * This file implements the necessary callbacks from the ST provided driver.
 * They are mere forwarders to the driver instance.
 */

#include <hal.h>
#include <hci.h>
#include <stm32_bluenrg_ble.h>
#include <tos/device/spbtlerf/adapter.hpp>
#include <tos/ft.hpp>

namespace tos::device::spbtle {
tos::any_alarm* get_alarm() {
    return adapter::instance()->cb_alarm();
}
}

extern "C" {
void Hal_Write_Serial(const void* data1,
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
    tos::device::spbtle::adapter::instance()->cb_reset();
    //    tos::spbtle::spbtle_rf::get(0)->reset();
}

uint8_t BlueNRG_DataPresent(void) {
    return tos::device::spbtle::adapter::instance()->cb_data_present();
    //    return tos::spbtle::spbtle_rf::get(0)->data_present();
}

void BlueNRG_HW_Bootloader(void) {
    tos::device::spbtle::adapter::instance()->cb_bootloader();
    //    tos::spbtle::spbtle_rf::get(0)->bootloader();
}

int32_t BlueNRG_SPI_Read_All(uint8_t* buffer, uint8_t buff_size) {
    return tos::device::spbtle::adapter::instance()->cb_spi_read({buffer, buff_size});
}

int32_t
BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2) {
    return tos::device::spbtle::adapter::instance()->cb_spi_write({data1, Nb_bytes1},
                                                                  {data2, Nb_bytes2});
}

void Enable_SPI_IRQ(void) {
    tos::device::spbtle::adapter::instance()->cb_spi_enable_irq();
    //    tos::spbtle::isr_enabled = true;
}

void Disable_SPI_IRQ(void) {
    tos::device::spbtle::adapter::instance()->cb_spi_disable_irq();
    //    tos::spbtle::isr_enabled = false;
}

void HCI_Event_CB(void* pckt) {
    tos::device::spbtle::adapter::instance()->cb_hci_event_call(pckt);
    //    HCI_callback(pckt);
}
}