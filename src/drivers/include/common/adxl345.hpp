//
// Created by fatih on 8/24/18.
//

#pragma once

#include <common/adxl345/constants.hpp>
#include <common/i2c.hpp>
#include <stdint.h>
#include <tos/debug/debug.hpp>
#include <tos/span.hpp>

namespace tos {
template<class TwimT>
class adxl345 {
public:
    explicit adxl345(TwimT twim);

    bool powerOn();

    struct accel {
        int x, y, z;
    };
    accel read() {
        accel res;
        readAccel(&res.x, &res.y, &res.z);
        return res;
    }

    void setRangeSetting(int val);

private:
    void readAccel(int* x, int* y, int* z);

    bool writeTo(uint8_t address, uint8_t val) {
        return writeToI2C(address, val);
    }
    bool readFrom(uint8_t address, int num, uint8_t buff[]) {
        return readFromI2C(address, num, buff);
    }
    bool readFromI2C(uint8_t address, int num, uint8_t* buff);
    bool writeToI2C(uint8_t address, uint8_t val);
    TwimT m_twim;
};
} // namespace tos


// impl
namespace tos {
template<class TwimT>
adxl345<TwimT>::adxl345(TwimT twim)
    : m_twim{twim} {
}

template<class TwimT>
bool adxl345<TwimT>::powerOn() {
    writeTo(ADXL345_POWER_CTL, 0);  // Wakeup
    writeTo(ADXL345_POWER_CTL, 16); // Auto_Sleep
    writeTo(ADXL345_POWER_CTL, 8);  // Measure
    return true;
}

template<class TwimT>
void adxl345<TwimT>::setRangeSetting(int val) {
    uint8_t _s;
    uint8_t _b;

    switch (val) {
    case 2:
        _s = 0b00000000;
        break;
    case 4:
        _s = 0b00000001;
        break;
    case 8:
        _s = 0b00000010;
        break;
    case 16:
        _s = 0b00000011;
        break;
    default:
        _s = 0b00000000;
    }
    readFrom(ADXL345_DATA_FORMAT, 1, &_b);
    _s |= (_b & 0b11101100);
    writeTo(ADXL345_DATA_FORMAT, _s);
}

template<class TwimT>
void adxl345<TwimT>::readAccel(int* x, int* y, int* z) {
    uint8_t buff[6];
    readFrom(ADXL345_DATAX0, ADXL345_TO_READ, buff); // Read Accel Data from ADXL345

    // Each Axis @ All g Ranges: 10 Bit Resolution (2 Bytes)
    *x = (int16_t)((((int)buff[1]) << 8) | buff[0]);
    *y = (int16_t)((((int)buff[3]) << 8) | buff[2]);
    *z = (int16_t)((((int)buff[5]) << 8) | buff[4]);
}

template<class TwimT>
bool adxl345<TwimT>::readFromI2C(uint8_t address, int num, uint8_t* buff) {
    auto wres = m_twim->transmit({ADXL345_DEVICE}, tos::monospan(address));
    if (wres != tos::twi_tx_res::ok) {
        // TODO: ERR
        return false;
    }
    auto res = m_twim->receive({ADXL345_DEVICE}, tos::span<uint8_t>(buff, num));
    if (res != tos::twi_rx_res::ok) {
        tos_debug_print("read error! %d", int(res));

        // TODO: ERR
        return false;
    }
    return true;
}

template<class TwimT>
bool adxl345<TwimT>::writeToI2C(uint8_t address, uint8_t val) {
    uint8_t buf[] = {address, val};
    auto res = m_twim->transmit({ADXL345_DEVICE}, buf);
    if (res != tos::twi_tx_res::ok) {
        tos_debug_print("write error! %d", int(res));
        // TODO: ERR
        return false;
    }
    return true;
}
} // namespace tos
