//
// Created by fatih on 8/24/18.
//

#pragma once

#include <arch/lx106/twim.hpp>
#include <stdint.h>

namespace tos
{
    class adxl345
    {
    public:
        explicit adxl345(esp82::twim& twim);

        void powerOn();

        void readAccel(int *x, int *y, int *z);

        void setRangeSetting(int val);
    private:
        void writeTo(uint8_t address, uint8_t val) { return writeToI2C(address, val); }
        void readFrom(uint8_t address, int num, uint8_t buff[]) { return readFromI2C(address, num, buff); }
        void readFromI2C(uint8_t address, int num, uint8_t* buff);
        void writeToI2C(uint8_t address, uint8_t val);
        esp82::twim& m_twim;
    };
} // namespace tos