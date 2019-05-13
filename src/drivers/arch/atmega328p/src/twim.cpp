//
// Created by fatih on 11/8/18.
//

#include <arch/twim.hpp>
#include <tos/debug.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <tos/semaphore.hpp>

#ifndef TWI_FREQ
#define TWI_FREQ 100000UL
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

enum class stat
{
    ok,
    addr_nack,
    data_nack,
};

namespace {
    static struct{
        uint8_t buffer[TWI_BUFFER_LENGTH];
        uint8_t length;
        uint8_t index;

        stat status;

        tos::semaphore sem{0};
    } transmission;

    void twi_init() {
        TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
        TWSR = 0; // prescaler = 1

        TWCR = _BV(TWEN);
    }

    void twi_start() {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
    }

    void twi_stop() {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
    }

    void twi_ack() {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
    }

    void twi_nack() {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
    }

    void twi_send(uint8_t data) {
        TWDR = data;
    }

    void twi_recv() {
        transmission.buffer[transmission.index++] = TWDR;
    }

    void twi_reply() {
        if (transmission.index < (transmission.length - 1)) {
            twi_ack();
        } else {
            twi_nack();
        }
    }

    void twi_done() {
        transmission.sem.up_isr();
    }

    void twi_write(uint8_t address, const uint8_t *data, uint8_t length) {
        transmission.buffer[0] = (address << 1) | TW_WRITE;
        transmission.length = length + 1;
        transmission.index = 0;
        memcpy(&transmission.buffer[1], data, length);

        twi_start();
    }

    void twi_read(uint8_t address, uint8_t length) {
        transmission.buffer[0] = (address << 1) | TW_READ;
        transmission.length = length + 1;
        transmission.index = 0;

        twi_start();
    }
}


namespace tos
{
namespace avr
{
    twim::twim(gpio::pin_type clock_pin, gpio::pin_type data_pin) {
        using namespace tos::tos_literals;
        if (data_pin != 18_pin && clock_pin != 19_pin)
        {
            //raise("atmega328p has fixed pins for I2C");
        }
        twi_init();
    }

    twi_tx_res twim::transmit(tos::twi_addr_t to, tos::span<const char> buf) noexcept {
        twi_write(to.addr, (const uint8_t*)buf.data(), buf.size());
        kern::busy();
        transmission.sem.down();
        kern::unbusy();
        return transmission.index == transmission.length ? twi_tx_res::ok : twi_tx_res::other;
    }

    twi_rx_res twim::receive(tos::twi_addr_t from, tos::span<char> buf) noexcept {
        twi_read(from.addr, buf.size());
        kern::busy();
        transmission.sem.down();
        kern::unbusy();
        uint8_t *data = &transmission.buffer[1];
        std::memcpy(buf.data(), data, std::min<size_t>(buf.size(), TWI_BUFFER_LENGTH - 1));
        return transmission.index == transmission.length ? twi_rx_res::ok : twi_rx_res::other;
    }
}
}

ISR(TWI_vect) {
    switch (TW_STATUS) {
        case TW_START:
        case TW_REP_START:
        case TW_MT_SLA_ACK:
        case TW_MT_DATA_ACK:
            if (transmission.index < transmission.length) {
                twi_send(transmission.buffer[transmission.index++]);
                twi_nack();
            } else {
                twi_stop();
                twi_done();
            }
            break;

        case TW_MR_DATA_ACK:
            twi_recv();
            twi_reply();
            break;

        case TW_MR_SLA_ACK:
            twi_reply();
            break;

        case TW_MR_DATA_NACK:
            twi_recv();
            twi_stop();
            twi_done();
            break;

        case TW_MT_SLA_NACK:
        case TW_MR_SLA_NACK:
        case TW_MT_DATA_NACK:
        default:
            twi_stop();
            twi_done();
            break;
    }
}