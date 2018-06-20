//
// Created by fatih on 5/8/18.
//

#pragma once

#include <arch/avr/gpio.hpp>

namespace tos
{
#define DHTLIB_DHT11_WAKEUP         18
#define DHTLIB_DHT_WAKEUP           1

#define DHTLIB_DHT11_LEADING_ZEROS  1
#define DHTLIB_DHT_LEADING_ZEROS    6

    enum class dht_res
    {
        ok,
        checksum = -1,
        timeout = - 2,
        connect = - 3,
        ack_l = -4,
        ack_h = -5
    };

// max timeout is 100 usec.
// For a 16 Mhz proc 100 usec is 1600 clock cycles
// loops using DHTLIB_TIMEOUT use at least 4 clock cycli
// so 100 us takes max 400 loops
// so by dividing F_CPU by 40000 we "fail" as fast as possible
#ifndef F_CPU
#define DHTLIB_TIMEOUT 1000  // ahould be approx. clock/40000
#else
#define DHTLIB_TIMEOUT (F_CPU/40000)
#endif

    class dht
    {
    public:
        dht() { m_disableIRQ = true; };

        dht_res read11(pin_t pin);
        dht_res read(pin_t pin);
        dht_res read12(pin_t pin);

        float humidity;
        float temperature;

    private:
        uint8_t bits[5];  // buffer to receive data
        dht_res read_sensor(pin_t pin, uint8_t wakeupDelay, uint8_t leadingZeroBits);
        bool   m_disableIRQ;
    };
}