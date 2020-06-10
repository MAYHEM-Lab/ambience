#pragma once

#include "detail/hts221_registers.hpp"

#include <common/i2c.hpp>
#include <tos/debug/log.hpp>
#include <tos/expected.hpp>
#include <tos/span.hpp>

namespace tos::device::hts221 {
enum class error_codes
{
    bad_identity,
    sensor_not_ready
};

using errors = mpark::variant<error_codes, i2c::errors>;

template<class I2C>
class driver {
public:
    explicit driver(I2C i2c, i2c::address7 address)
        : m_i2c(std::move(i2c))
        , m_address{address} {
        calibrate();
    }

    friend expected<driver, errors> open(I2C i2c, i2c::address7 address);

    expected<void, errors> calibrate();

    expected<void, errors> power_up();

    expected<void, errors> power_down();

    expected<double, errors> get_temperature();
    expected<double, errors> get_humidity();

    expected<void, errors> trigger_measurement();

    expected<void, errors> set_data_rate(detail::HTS221DataRate_t dataRate);

    expected<std::pair<bool, bool>, errors> get_ready() const;

private:
    I2C m_i2c;
    i2c::address7 m_address;
    int32_t H0_RH_X2, H1_RH_X2, T0_DEGC_X8, T1_DEGC_X8, T1_T0_MSB, H0_T0, H1_T0, T0_OUT,
        T1_OUT;
};
} // namespace tos::device::hts221

// Implementation

namespace tos::device::hts221 {
template<class I2C>
expected<driver<I2C>, errors> open(I2C i2c, i2c::address7 address) {
    using namespace detail;

    auto data = TRY(i2c::read_byte_data(i2c, address, HTS221_REGISTER_WHO_AM_I));
    if (data != 0xBC) {
        return unexpected(errors(error_codes::bad_identity));
    }

    auto d = driver<I2C>(std::move(i2c), address);

    d.power_down();

    data = 0;

    data |= HTS221_BDU_ENABLE;
    data |= HTS221_DATARATE_1_HZ;

    TRYV(i2c::write_byte_data(i2c, address, HTS221_REGISTER_CTRL_REG1, data));

    data = 0;

    data |= HTS221_AVERAGE_HUMIDITY_4_SAMPLES;
    data |= HTS221_AVERAGE_TEMPERATURE_2_SAMPLES;

    TRYV(i2c::write_byte_data(i2c, address, HTS221_REGISTER_AV_CONF, data));

    return d;
}

template<class I2C>
expected<void, errors> driver<I2C>::calibrate() {
    using namespace detail;
    int32_t data = 0;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H0_RH_X2));
    H0_RH_X2 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H1_RH_X2));
    H1_RH_X2 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T0_DEGC_X8));
    T0_DEGC_X8 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T1_T0_MSB));
    T0_DEGC_X8 |= (data & 0x03) << 8;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T1_DEGC_X8));
    T1_DEGC_X8 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T1_T0_MSB));
    T1_DEGC_X8 |= (data & 0x0C) << 6;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H0_T0_OUT_L));
    H0_T0 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H0_T0_OUT_H));
    H0_T0 |= data << 8;

    if (H0_T0 > 32768) {
        H0_T0 -= 65536;
    }

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H1_T0_OUT_L));
    H1_T0 = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_H1_T0_OUT_H));
    H1_T0 |= data << 8;

    if (H1_T0 > 32768) {
        H1_T0 -= 65536;
    }

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T0_OUT_L));
    T0_OUT = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T0_OUT_H));
    T0_OUT |= data << 8;

    if (T0_OUT > 32768) {
        T0_OUT -= 65536;
    }

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T1_OUT_L));
    T1_OUT = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CALIB_T1_OUT_H));
    T1_OUT |= data << 8;

    if (T1_OUT > 32768) {
        T1_OUT -= 65536;
    }

    LOG_TRACE(H0_RH_X2,
              H1_RH_X2,
              T0_DEGC_X8,
              T1_DEGC_X8,
              T1_T0_MSB,
              H0_T0,
              H1_T0,
              T0_OUT,
              T1_OUT);

    return {};
}

template<class I2C>
expected<void, errors> driver<I2C>::power_up() {
    using namespace detail;
    int32_t data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1));

    data |= HTS221_POWER_UP;

    TRYV(
        i2c::write_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1, data));

    return {};
}

template<class I2C>
expected<void, errors> driver<I2C>::power_down() {
    using namespace detail;
    int32_t data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1));

    data &= ~HTS221_POWER_UP;

    TRYV(
        i2c::write_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1, data));

    return {};
}

template<class I2C>
expected<std::pair<bool, bool>, errors> driver<I2C>::get_ready() const {
    using namespace detail;
    auto data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_STATUS_REG));
    return std::make_pair((data & 0b01) == 0b01, (data & 0b10) == 0b10);
}

template<class I2C>
expected<double, errors> driver<I2C>::get_temperature() {
    using namespace detail;

    int32_t data = 0;
    int32_t temperature = 0;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_TEMP_OUT_L));
    temperature = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_TEMP_OUT_H));
    temperature |= data << 8;

    if (temperature > 32768) {
        temperature -= 65536;
    }

    return T0_DEGC_X8 / 8.0 +
           (temperature - T0_OUT) * (T1_DEGC_X8 - T0_DEGC_X8) / 8.0 / (T1_OUT - T0_OUT);
}

template<class I2C>
expected<double, errors> driver<I2C>::get_humidity() {
    using namespace detail;
    int32_t data = 0;
    int32_t humidity = 0;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_HUMIDITY_OUT_L));
    humidity = data;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_HUMIDITY_OUT_H));
    humidity |= data << 8;

    if (humidity > 32768) {
        humidity -= 65536;
    }

    return H0_RH_X2 / 2.0 +
           (humidity - H0_T0) * (H1_RH_X2 - H0_RH_X2) / 2.0 / (H1_T0 - H0_T0);
}

template<class I2C>
expected<void, errors> driver<I2C>::trigger_measurement() {
    using namespace detail;

    int32_t data = 0;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1));

    if ((data & 0b11) == HTS221_DATARATE_ONE_SHOT) {
        TRYV(i2c::write_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG2, 0b1));
    } else {
        LOG_WARN("No need to trigger");
    }

    return {};
}

template<class I2C>
expected<void, errors> driver<I2C>::set_data_rate(detail::HTS221DataRate_t dataRate) {
    using namespace detail;
    int32_t data = 0;

    data = TRY(i2c::read_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1));

    data &= ~0b11;
    data |= dataRate;

    TRYV(i2c::write_byte_data(m_i2c, m_address, HTS221_REGISTER_CTRL_REG1, data));

    return {};
}
} // namespace tos::device::hts221