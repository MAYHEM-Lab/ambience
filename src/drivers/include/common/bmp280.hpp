//
// Created by fatih on 1/18/19.
//

#pragma once

#include <common/i2c.hpp>
#include <cstdint>

namespace tos
{
namespace detail
{
    struct bmp280_calib_data
    {
        /** dig_T1 cal register. */
        uint16_t dig_T1;
        /** dig_T2 cal register. */
        int16_t  dig_T2;
        /** dig_T3 cal register. */
        int16_t  dig_T3;

        /** dig_P1 cal register. */
        uint16_t dig_P1;
        /** dig_P2 cal register. */
        int16_t  dig_P2;
        /** dig_P3 cal register. */
        int16_t  dig_P3;
        /** dig_P4 cal register. */
        int16_t  dig_P4;
        /** dig_P5 cal register. */
        int16_t  dig_P5;
        /** dig_P6 cal register. */
        int16_t  dig_P6;
        /** dig_P7 cal register. */
        int16_t  dig_P7;
        /** dig_P8 cal register. */
        int16_t  dig_P8;
        /** dig_P9 cal register. */
        int16_t  dig_P9;

        /** dig_H1 cal register. */
        uint8_t  dig_H1;
        /** dig_H2 cal register. */
        int16_t  dig_H2;
        /** dig_H3 cal register. */
        uint8_t  dig_H3;
        /** dig_H4 cal register. */
        int16_t  dig_H4;
        /** dig_H5 cal register. */
        int16_t  dig_H5;
        /** dig_H6 cal register. */
        int8_t   dig_H6;
    };
} // namespace detail

enum class sensor_sampling {
    /** No over-sampling. */
    SAMPLING_NONE = 0x00,
    /** 1x over-sampling. */
    SAMPLING_X1   = 0x01,
    /** 2x over-sampling. */
    SAMPLING_X2   = 0x02,
    /** 4x over-sampling. */
    SAMPLING_X4   = 0x03,
    /** 8x over-sampling. */
    SAMPLING_X8   = 0x04,
    /** 16x over-sampling. */
    SAMPLING_X16  = 0x05
};

/** Operating mode for the sensor. */
enum class sensor_mode {
    /** Sleep mode. */
    MODE_SLEEP  = 0x00,
    /** Forced mode. */
    MODE_FORCED = 0x01,
    /** Normal mode. */
    MODE_NORMAL = 0x03,
    /** Software reset. */
    MODE_SOFT_RESET_CODE = 0xB6
};

/** Filtering level for sensor data. */
enum class sensor_filter {
    /** No filtering. */
    FILTER_OFF = 0x00,
    /** 2x filtering. */
    FILTER_X2  = 0x01,
    /** 4x filtering. */
    FILTER_X4  = 0x02,
    /** 8x filtering. */
    FILTER_X8  = 0x03,
    /** 16x filtering. */
    FILTER_X16 = 0x04
};

/** Standby duration in ms */
enum class standby_duration {
    /** 1 ms standby. */
    STANDBY_MS_1      = 0x00,
    /** 63 ms standby. */
    STANDBY_MS_63     = 0x01,
    /** 125 ms standby. */
    STANDBY_MS_125    = 0x02,
    /** 250 ms standby. */
    STANDBY_MS_250    = 0x03,
    /** 500 ms standby. */
    STANDBY_MS_500    = 0x04,
    /** 1000 ms standby. */
    STANDBY_MS_1000   = 0x05,
    /** 2000 ms standby. */
    STANDBY_MS_2000   = 0x06,
    /** 4000 ms standby. */
    STANDBY_MS_4000   = 0x07
};

template <class TwimT>
class bmp280
{
public:

    bmp280(TwimT twim, twi_addr_t addr, uint8_t chip_id = 0x58);

    void setSampling(sensor_mode mode = sensor_mode::MODE_NORMAL,
                     sensor_sampling tempSampling  = sensor_sampling::SAMPLING_X16,
                     sensor_sampling pressSampling = sensor_sampling::SAMPLING_X16,
                     sensor_filter filter          = sensor_filter::FILTER_OFF,
                     standby_duration duration     = standby_duration::STANDBY_MS_1);

    float readTemperature()
    {
        int32_t var1, var2;

        int32_t adc_T = read24(BMP280_REGISTER_TEMPDATA);
        adc_T >>= 4;

        var1  = ((((adc_T>>3) - ((int32_t)_bmp280_calib.dig_T1 <<1))) *
                 ((int32_t)_bmp280_calib.dig_T2)) >> 11;

        var2  = (((((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1)) *
                   ((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) *
                 ((int32_t)_bmp280_calib.dig_T3)) >> 14;

        t_fine = var1 + var2;

        float T  = (t_fine * 5 + 128) >> 8;
        return T/100;
    }

private:
    void write8(uint8_t reg, uint8_t value)
    {
        const char buf[2] = {reg, value};
        m_twim->transmit(m_addr, buf);
    }

    uint8_t read8(uint8_t reg)
    {
        m_twim->transmit(m_addr, { reinterpret_cast<const char*>(&reg), 1 });

        uint8_t value;
        m_twim->receive(m_addr, {reinterpret_cast<const char*>(&value), 1 });

        return value;
    }

    uint16_t read16(uint8_t reg)
    {
        m_twim->transmit(m_addr, span<const char>{ reinterpret_cast<const char*>(&reg), 1 });

        uint8_t value[2];
        m_twim->receive(m_addr, span<char>{reinterpret_cast<char*>(&value), 2 });

        return ((uint16_t)value[0] << 8) | value[1];
    }

    uint16_t read16_LE(uint8_t reg) {
        uint16_t temp = read16(reg);
        return (temp >> 8) | (temp << 8);
    }

    int16_t readS16(uint8_t reg)
    {
        return (int16_t)read16(reg);
    }

    int16_t readS16_LE(uint8_t reg)
    {
        return (int16_t)read16_LE(reg);
    }

    uint32_t read24(uint8_t reg)
    {
        uint8_t value[3];

        m_twim->transmit(m_addr, tos::span<const char>{ reinterpret_cast<const char*>(&reg), 1 });

        m_twim->receive(m_addr, tos::span<char>{reinterpret_cast<char*>(&value), 3 });

        return ((uint32_t)value[0] << 16) | (value[1] << 8) | value[2];
    }

    enum
    {
        BMP280_REGISTER_DIG_T1              = 0x88,
        BMP280_REGISTER_DIG_T2              = 0x8A,
        BMP280_REGISTER_DIG_T3              = 0x8C,
        BMP280_REGISTER_DIG_P1              = 0x8E,
        BMP280_REGISTER_DIG_P2              = 0x90,
        BMP280_REGISTER_DIG_P3              = 0x92,
        BMP280_REGISTER_DIG_P4              = 0x94,
        BMP280_REGISTER_DIG_P5              = 0x96,
        BMP280_REGISTER_DIG_P6              = 0x98,
        BMP280_REGISTER_DIG_P7              = 0x9A,
        BMP280_REGISTER_DIG_P8              = 0x9C,
        BMP280_REGISTER_DIG_P9              = 0x9E,
        BMP280_REGISTER_CHIPID              = 0xD0,
        BMP280_REGISTER_VERSION             = 0xD1,
        BMP280_REGISTER_SOFTRESET           = 0xE0,
        BMP280_REGISTER_CAL26               = 0xE1,  // R calibration = 0xE1-0xF0
        BMP280_REGISTER_CONTROL             = 0xF4,
        BMP280_REGISTER_CONFIG              = 0xF5,
        BMP280_REGISTER_PRESSUREDATA        = 0xF7,
        BMP280_REGISTER_TEMPDATA            = 0xFA,
    };

    void readCoefficients()
    {
        _bmp280_calib.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1);
        _bmp280_calib.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2);
        _bmp280_calib.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3);

        _bmp280_calib.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1);
        _bmp280_calib.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2);
        _bmp280_calib.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3);
        _bmp280_calib.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4);
        _bmp280_calib.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5);
        _bmp280_calib.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6);
        _bmp280_calib.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7);
        _bmp280_calib.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8);
        _bmp280_calib.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9);
    }

    twi_addr_t m_addr;
    TwimT m_twim;

    struct config
    {
        /** Inactive duration (standby time) in normal mode */
        unsigned int t_sb : 3;
        /** Filter settings */
        unsigned int filter : 3;
        /** Unused - don't set */
        unsigned int none : 1;
        /** Enables 3-wire SPI */
        unsigned int spi3w_en : 1;
        /** Used to retrieve the assembled config register's byte value. */
        unsigned int get()
        {
            return (t_sb << 5) | (filter << 3) | spi3w_en;
        }
    };

    /** Encapsulates trhe ctrl_meas register */
    struct ctrl_meas
    {
        /** Temperature oversampling. */
        unsigned int osrs_t : 3;
        /** Pressure oversampling. */
        unsigned int osrs_p : 3;
        /** Device mode */
        unsigned int mode : 2;
        /** Used to retrieve the assembled ctrl_meas register's byte value. */
        unsigned int get()
        {
            return (osrs_t << 5) | (osrs_p << 3) | mode;
        }
    };

    int32_t   _sensorID;
    int32_t   t_fine;

    detail::bmp280_calib_data _bmp280_calib;
    config    _configReg;
    ctrl_meas _measReg;
};
} // namespace tos

// impl
namespace tos
{
    template<class TwimT>
    bmp280<TwimT>::bmp280(TwimT twim, twi_addr_t addr, uint8_t chip_id)
        : m_twim{std::move(twim)}, m_addr{addr}, _sensorID{chip_id} {
        readCoefficients();
        setSampling();
    }

    template<class TwimT>
    void bmp280<TwimT>::setSampling(sensor_mode mode, sensor_sampling tempSampling, sensor_sampling pressSampling,
                                    sensor_filter filter, standby_duration duration) {
        _measReg.mode     = (uint8_t)mode;
        _measReg.osrs_t   = (uint8_t)tempSampling;
        _measReg.osrs_p   = (uint8_t)pressSampling;

        _configReg.filter = (uint8_t)filter;
        _configReg.t_sb   = (uint8_t)duration;

        write8(BMP280_REGISTER_CONFIG, _configReg.get());
        write8(BMP280_REGISTER_CONTROL, _measReg.get());
    }
}