//
// Created by fatih on 5/8/18.
//

#include <common/dht22.hpp>
//#include <util/delay.h>
#include <tos/print.hpp>
#include <tos/interrupt.hpp>

namespace tos
{
    dht_res dht::read11(esp82::pin_t pin)
    {
        // READ VALUES
        if (m_disableIRQ) kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT11_WAKEUP, DHTLIB_DHT11_LEADING_ZEROS);
        if (m_disableIRQ) kern::enable_interrupts();

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

    dht_res dht::read12(esp82::pin_t pin)
    {
        // READ VALUES
        if (m_disableIRQ) kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT11_WAKEUP, DHTLIB_DHT11_LEADING_ZEROS);
        if (m_disableIRQ) kern::enable_interrupts();

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

    dht_res dht::read(esp82::pin_t pin)
    {
        // READ VALUES
        if (m_disableIRQ) kern::disable_interrupts();
        auto result = read_sensor(pin, DHTLIB_DHT_WAKEUP, DHTLIB_DHT_LEADING_ZEROS);
        if (m_disableIRQ) kern::enable_interrupts();

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
        uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
        if (bits[4] != sum)
        {
            return dht_res::checksum;
        }
        return result;
    }

    tos::esp82::gpio g;
    dht_res dht::read_sensor(esp82::pin_t pin, uint8_t wakeupDelay, uint8_t leadingZeroBits)
    {
        // INIT BUFFERVAR TO RECEIVE DATA
        uint8_t mask = 128;
        uint8_t idx = 0;

        uint8_t data = 0;
        uint8_t state = false;
        uint8_t pstate = false;
        uint16_t zeroLoop = DHTLIB_TIMEOUT;
        uint16_t delta = 0;

        leadingZeroBits = 40 - leadingZeroBits;

        // REQUEST SAMPLE
        g.set_pin_mode(pin, pin_mode::out);
        g.write(pin, false);
        //pinMode(pin, OUTPUT);
        //digitalWrite(pin, LOW); // T-be
        //if (wakeupDelay > 8) ets_delay_us(wakeupDelay * 1000UL);
        //else
            ets_delay_us(wakeupDelay * 1000UL);
        // digitalWrite(pin, HIGH); // T-go
        //pinMode(pin, INPUT);
        g.set_pin_mode(pin, pin_mode::in);

        uint16_t loopCount = DHTLIB_TIMEOUT * 2;  // 200uSec max
        // while(digitalRead(pin) == HIGH)
        while (g.read(pin) != false)
        {
            if (--loopCount == 0)
            {
                return dht_res::connect;
            }
        }

        // GET ACKNOWLEDGE or TIMEOUT
        loopCount = DHTLIB_TIMEOUT;
        // while(digitalRead(pin) == LOW)
        while (g.read(pin) == false)  // T-rel
        {
            if (--loopCount == 0)
            {
                return dht_res::ack_l;
            }
        }

        loopCount = DHTLIB_TIMEOUT;
        // while(digitalRead(pin) == HIGH)
        while (g.read(pin) != false)  // T-reh
        {
            if (--loopCount == 0)
            {
                return dht_res::ack_h;
            }
        }

        loopCount = DHTLIB_TIMEOUT;

        // READ THE OUTPUT - 40 BITS => 5 BYTES
        for (uint8_t i = 40; i != 0; )
        {
            // WAIT FOR FALLING EDGE
            state = g.read(pin);
            if (state == false && pstate != false)
            {
                if (i > leadingZeroBits) // DHT22 first 6 bits are all zero !!   DHT11 only 1
                {
                    zeroLoop = (zeroLoop < loopCount ? zeroLoop : loopCount);
                    delta = (DHTLIB_TIMEOUT - zeroLoop)/4;
                }
                else if ( loopCount <= (zeroLoop - delta) ) // long -> one
                {
                    data |= mask;
                }
                mask >>= 1;
                if (mask == 0)   // next byte
                {
                    mask = 128;
                    bits[idx] = data;
                    idx++;
                    data = 0;
                }
                // next bit
                --i;

                // reset timeout flag
                loopCount = DHTLIB_TIMEOUT;
            }
            pstate = state;
            // Check timeout
            if (--loopCount == 0)
            {
                return dht_res::timeout;
            }

        }
        return dht_res::ok;
    }
}