#include <arch/gpio.hpp>
#include <tos/ft.hpp>

extern "C" {
#include <user_interface.h>
}
using namespace tos::esp82;

unsigned int preferred_si2c_clock = 100000;
unsigned char twi_dcount = 18;
static pin_t pin_sda, pin_scl;
static uint32_t twi_clockStretchLimit;

inline void SDA_LOW() { gpio::write(pin_sda, tos::digital::low); }
inline void SDA_HIGH() { gpio::write(pin_sda, tos::digital::high); }
inline auto SDA_READ() { return gpio::read(pin_sda); }

inline void SCL_LOW() { gpio::write(pin_scl, tos::digital::low); }
inline void SCL_HIGH() { gpio::write(pin_scl, tos::digital::high); }
inline auto SCL_READ() { return gpio::read(pin_scl); }

#ifndef FCPU80
#define FCPU80 80000000L
#endif
#define F_CPU FCPU80

#if F_CPU == FCPU80
#define TWI_CLOCK_STRETCH_MULTIPLIER 3
#else
#define TWI_CLOCK_STRETCH_MULTIPLIER 6
#endif

void twi_setClock(unsigned int freq) {
    preferred_si2c_clock = freq;
#if F_CPU == FCPU80
    if (freq <= 50000)
        twi_dcount = 38; // about 50KHz
    else if (freq <= 100000)
        twi_dcount = 19; // about 100KHz
    else if (freq <= 200000)
        twi_dcount = 8; // about 200KHz
    else if (freq <= 300000)
        twi_dcount = 3; // about 300KHz
    else if (freq <= 400000)
        twi_dcount = 1; // about 400KHz
    else
        twi_dcount = 1; // about 400KHz
#else
    if (freq <= 50000)
        twi_dcount = 64; // about 50KHz
    else if (freq <= 100000)
        twi_dcount = 32; // about 100KHz
    else if (freq <= 200000)
        twi_dcount = 14; // about 200KHz
    else if (freq <= 300000)
        twi_dcount = 8; // about 300KHz
    else if (freq <= 400000)
        twi_dcount = 5; // about 400KHz
    else if (freq <= 500000)
        twi_dcount = 3; // about 500KHz
    else if (freq <= 600000)
        twi_dcount = 2; // about 600KHz
    else
        twi_dcount = 1; // about 700KHz
#endif
}

void twi_setClockStretchLimit(uint32_t limit) {
    twi_clockStretchLimit = limit * TWI_CLOCK_STRETCH_MULTIPLIER;
}

void twi_init(unsigned char sda, unsigned char scl) {
    using namespace tos::esp82;
    pin_sda = pin_t{sda};
    pin_scl = pin_t{scl};

    gpio::set_pin_mode(pin_sda, tos::pin_mode::in_pullup);
    gpio::set_pin_mode(pin_scl, tos::pin_mode::in_pullup);

    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(sda)),
                   GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(sda))) |
                       GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); // open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << sda));
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(scl)),
                   GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(scl))) |
                       GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); // open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << scl));

    twi_setClock(preferred_si2c_clock);
    twi_setClockStretchLimit(230); // default value is 230 uS
}

void twi_stop() {
    gpio::set_pin_mode(pin_sda, tos::pin_mode::in);
    gpio::set_pin_mode(pin_scl, tos::pin_mode::in);
}

#define ESP8266_REG(addr) *((volatile uint32_t*)(0x60000000 + (addr)))
#define GPI ESP8266_REG(0x318) // GPIO_IN RO (Read Input Level)
static void twi_delay(unsigned char v) {
    unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    unsigned char reg;
    for (i = 0; i < v; i++) reg = GPI;
#pragma GCC diagnostic pop
}

#undef GPI

static bool twi_write_start(void) {
    SCL_HIGH();
    SDA_HIGH();
    if (SDA_READ() == 0)
        return false;
    twi_delay(twi_dcount);
    SDA_LOW();
    twi_delay(twi_dcount);
    return true;
}

static bool twi_write_stop(void) {
    uint32_t i = 0;
    SCL_LOW();
    SDA_LOW();
    twi_delay(twi_dcount);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < twi_clockStretchLimit)
        ; // Clock stretching
    twi_delay(twi_dcount);
    SDA_HIGH();
    twi_delay(twi_dcount);

    return true;
}

static bool twi_write_bit(bool bit) {
    uint32_t i = 0;
    SCL_LOW();
    if (bit)
        SDA_HIGH();
    else
        SDA_LOW();
    twi_delay(twi_dcount + 1);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < twi_clockStretchLimit)
        ; // Clock stretching
    twi_delay(twi_dcount);
    return true;
}

static bool twi_read_bit(void) {
    uint32_t i = 0;
    SCL_LOW();
    SDA_HIGH();
    twi_delay(twi_dcount + 2);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < twi_clockStretchLimit)
        ; // Clock stretching
    bool bit = SDA_READ();
    twi_delay(twi_dcount);
    return bit;
}

static bool twi_write_byte(unsigned char byte) {
    unsigned char bit;
    for (bit = 0; bit < 8; bit++) {
        twi_write_bit(byte & 0x80);
        byte <<= 1;
    }
    return !twi_read_bit(); // NACK/ACK
}

static unsigned char twi_read_byte(bool nack) {
    unsigned char byte = 0;
    unsigned char bit;
    for (bit = 0; bit < 8; bit++) byte = (byte << 1) | twi_read_bit();
    twi_write_bit(nack);
    return byte;
}

unsigned char twi_writeTo(unsigned char address,
                          unsigned char* buf,
                          unsigned int len,
                          unsigned char sendStop) {
    unsigned int i;
    if (!twi_write_start())
        return 4; // line busy
    if (!twi_write_byte(((address << 1) | 0) & 0xFF)) {
        if (sendStop)
            twi_write_stop();
        return 2; // received NACK on transmit of address
    }
    for (i = 0; i < len; i++) {
        if (!twi_write_byte(buf[i])) {
            if (sendStop)
                twi_write_stop();
            return 3; // received NACK on transmit of data
        }
    }
    if (sendStop)
        twi_write_stop();
    i = 0;
    while (SDA_READ() == 0 && (i++) < 10) {
        SCL_LOW();
        twi_delay(twi_dcount);
        SCL_HIGH();
        twi_delay(twi_dcount);
    }

    tos::this_thread::yield();
    return 0;
}

unsigned char twi_readFrom(unsigned char address,
                           unsigned char* buf,
                           unsigned int len,
                           unsigned char sendStop) {
    unsigned int i;
    if (!twi_write_start())
        return 4; // line busy
    if (!twi_write_byte(((address << 1) | 1) & 0xFF)) {
        if (sendStop)
            twi_write_stop();
        return 2; // received NACK on transmit of address
    }
    for (i = 0; i < (len - 1); i++) {
        buf[i] = twi_read_byte(false);
    }
    buf[len - 1] = twi_read_byte(true);
    if (sendStop)
        twi_write_stop();
    i = 0;
    while (SDA_READ() == 0 && (i++) < 10) {
        SCL_LOW();
        twi_delay(twi_dcount);
        SCL_HIGH();
        twi_delay(twi_dcount);
    }

    tos::this_thread::yield();
    return 0;
}
#define I2C_OK 0
#define I2C_SCL_HELD_LOW 1
#define I2C_SCL_HELD_LOW_AFTER_READ 2
#define I2C_SDA_HELD_LOW 3
#define I2C_SDA_HELD_LOW_AFTER_INIT 4

uint8_t twi_status() {
    if (SCL_READ() == 0)
        return I2C_SCL_HELD_LOW; // SCL held low by another device, no procedure available
                                 // to recover
    int clockCount = 20;

    while (SDA_READ() == 0 &&
           clockCount > 0) { // if SDA low, read the bits slaves have to sent to a max
        --clockCount;
        twi_read_bit();
        if (SCL_READ() == 0)
            return I2C_SCL_HELD_LOW_AFTER_READ; // I2C bus error. SCL held low beyond
                                                // slave clock stretch time
    }

    if (SDA_READ() == 0)
        return I2C_SDA_HELD_LOW; // I2C bus error. SDA line held low by
                                 // slave/another_master after n bits.

    if (!twi_write_start())
        return I2C_SDA_HELD_LOW_AFTER_INIT; // line busy. SDA again held low by another
                                            // device. 2nd master?

    return I2C_OK; // all ok
}