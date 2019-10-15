//
// Created by fatih on 8/24/18.
//

#include <arch/twim.hpp>
#include <tos/interrupt.hpp>
#include <tos/thread.hpp>

extern "C" {
#include <user_interface.h>
}

#define I2C_OK 0
#define I2C_SCL_HELD_LOW 1
#define I2C_SCL_HELD_LOW_AFTER_READ 2
#define I2C_SDA_HELD_LOW 3
#define I2C_SDA_HELD_LOW_AFTER_INIT 4

void twi_init(tos::esp82::gpio& gpio, tos::esp82::pin_t sda, tos::esp82::pin_t scl);
void twi_stop();
void twi_setClock(unsigned int freq);
void twi_setClockStretchLimit(uint32_t limit);
uint8_t twi_writeTo(unsigned char address,
                    unsigned char* buf,
                    unsigned int len,
                    unsigned char sendStop);
uint8_t twi_readFrom(unsigned char address,
                     unsigned char* buf,
                     unsigned int len,
                     unsigned char sendStop);
uint8_t twi_status();

extern "C" int ets_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
namespace tos {
namespace esp82 {
twim::twim(gpio& gpio, tos::esp82::pin_t clock_pin, tos::esp82::pin_t data_pin) {
    twi_init(gpio, data_pin, clock_pin);
    // twi_setClock(400000);
}

twi_tx_res twim::transmit(twi_addr_t to, span<const char> buf) noexcept {
    auto res = twi_writeTo(to.addr, (unsigned char*)buf.data(), buf.size(), true);
    tos::this_thread::yield();
    switch (res) {
    case I2C_OK:
        return twi_tx_res::ok;
    default:
        return twi_tx_res::other;
    }
}

twi_rx_res twim::receive(twi_addr_t from, span<char> buf) noexcept {
    auto res = twi_readFrom(from.addr, (unsigned char*)buf.data(), buf.size(), true);
    tos::this_thread::yield();
    switch (res) {
    case I2C_OK:
        return twi_rx_res::ok;
    default:
        return twi_rx_res::other;
    }
}
} // namespace esp82
} // namespace tos