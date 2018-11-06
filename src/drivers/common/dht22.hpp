//
// Created by fatih on 5/8/18.
//

#pragma once

#include <tos/utility.hpp>
#include <chrono>
#include <tos/interrupt.hpp>
#include <drivers/common/gpio.hpp>

#include <tos/fixed_point.hpp>
#include <algorithm>

namespace tos
{
#define DHTLIB_DHT11_WAKEUP         18
#define DHTLIB_DHT_WAKEUP           20

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
#define DHTLIB_TIMEOUT 2000  // ahould be approx. clock/40000
#else
#define DHTLIB_TIMEOUT (F_CPU/40'000)
#endif

    template <class GpioT, class DelayT>
    class dht : private DelayT
    {
        using pin_t = typename GpioT::pin_type;
    public:

        explicit dht(GpioT& gpio, DelayT delay)
        : DelayT{std::move(delay)}, m_gpio{&gpio}
        {
        };

        dht(const dht&) = delete;

        dht(dht&&) = default;

        dht_res read11(pin_t pin);
        dht_res read(pin_t pin);
        dht_res read12(pin_t pin);

        float humidity;
        float temperature;
        uint8_t bits[5];  // buffer to receive data

    private:

        dht_res read_sensor(pin_t pin, uint8_t wakeupDelay, uint8_t leadingZeroBits);

        GpioT* m_gpio;

        uint32_t expectPulse(pin_t pin, bool level) {
            uint32_t count = 0;

            while (m_gpio->read(pin) == level) {
              if (count++ >= 8'000) {
                return 0; // Exceeded timeout, fail.
              }
            }

            return count;
        }
    };

    template <class GpioT, class DelayT>
    auto make_dht(GpioT &g, DelayT &&d)
    {
        using namespace std;
        using GT = remove_reference_t <remove_const_t <GpioT>>;
        using DT = remove_reference_t <remove_const_t <DelayT>>;
        return dht<GT, DT>(g, forward<DelayT>(d));
    }
}

// IMPL

namespace tos
{
    template <class GpioT, class DelayT>
    dht_res dht<GpioT, DelayT>::read_sensor(pin_t pin, uint8_t wakeupDelay, uint8_t)
    {
        std::fill(bits, bits + 5, 0);

        auto& g = *m_gpio;
        auto& delay = static_cast<DelayT&>(*this);

        g.set_pin_mode(pin, pin_mode::out);
        g.write(pin, digital::low);

        using namespace std::chrono_literals;
        delay(std::chrono::milliseconds{ wakeupDelay });

        g.write(pin, digital::high);
        delay(40us);

        g.set_pin_mode(pin, pin_mode::in_pullup);
        delay(10us);

        if (expectPulse(pin, false) == 0) {
            return dht_res::connect;
        }

        if (expectPulse(pin, true) == 0) {
            return dht_res::connect;
        }

        static uint32_t cycles[80];

        for (int i=0; i < 80; i+=2) {
            cycles[i]   = expectPulse(pin, false);
            cycles[i+1] = expectPulse(pin, true);
        }

        for (int i=0; i<40; ++i) {
            uint32_t lowCycles  = cycles[2*i];
            uint32_t highCycles = cycles[2*i+1];
            if ((lowCycles == 0) || (highCycles == 0)) {
                return dht_res::timeout;
            }
            bits[i/8] <<= 1;
            // Now compare the low and high cycle times to see if the bit is a 0 or 1.
            if (highCycles > lowCycles) {
                // High cycles are greater than 50us low cycle count, must be a 1.
                bits[i/8] |= 1;
            }
            // Else high cycles are less than (or equal to, a weird case) the 50us low
            // cycle count so this must be a zero.  Nothing needs to be changed in the
            // stored data.
        }

        return dht_res::ok;
    }

    template <class GpioT, class DelayT>
    dht_res dht<GpioT, DelayT>::read11(pin_t pin)
    {
        // READ VALUES
        kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT11_WAKEUP, DHTLIB_DHT11_LEADING_ZEROS);
        kern::enable_interrupts();

        // these bits are always zero, masking them reduces errors.
        bits[0] &= 0x7F;
        bits[2] &= 0x7F;

        // CONVERT AND STORE
        humidity    = bits[0];  // bits[1] == 0;
        temperature = bits[2];  // bits[3] == 0;

        // TEST CHECKSUM
        uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
        if (bits[4] != sum)
        {
            return dht_res::checksum;
        }
        return result;
    }

    template <class GpioT, class DelayT>
    dht_res dht<GpioT, DelayT>::read12(pin_t pin)
    {
        // READ VALUES
        kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT11_WAKEUP, DHTLIB_DHT11_LEADING_ZEROS);
        kern::enable_interrupts();

        // CONVERT AND STORE
        humidity = bits[0] + bits[1] * 0.1;
        temperature = (bits[2] & 0x7F) + bits[3] * 0.1;
        if (bits[2] & 0x80)  // negative temperature
        {
            temperature = -temperature;
        }

        // TEST CHECKSUM
        uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
        if (bits[4] != sum)
        {
            return dht_res::checksum;
        }
        return result;
    }

    template <class GpioT, class DelayT>
    dht_res dht<GpioT, DelayT>::read(pin_t pin)
    {
        // READ VALUES
        kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT_WAKEUP, DHTLIB_DHT_LEADING_ZEROS);
        kern::enable_interrupts();

        // these bits are always zero, masking them reduces errors.
        bits[0] &= 0x03;
        bits[2] &= 0x83;

        // CONVERT AND STORE
        humidity = (bits[0]*256 + bits[1]) * 0.1;
        temperature = ((bits[2] & 0x7F)*256 + bits[3]) * 0.1;
        if (bits[2] & 0x80)  // negative temperature
        {
            temperature = -temperature;
        }

        // TEST CHECKSUM
        uint8_t sum = uint8_t((bits[0] + bits[1] + bits[2] + bits[3]) & 0xFF);
        if (bits[4] != sum)
        {
            return dht_res::checksum;
        }
        return result;
    }
}

