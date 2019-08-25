//
// Created by fatih on 4/4/19.
//

#pragma once

#include <common/bme280/bme280.h>
#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <tos/expected.hpp>

namespace tos {
namespace bme280 {
enum class read_errors : uint8_t
{
    device_not_found = 254,
    communication_error = 252,
};

enum class components : uint8_t
{
    pressure = BME280_PRESS,
    temp = BME280_TEMP,
    humidity = BME280_HUM,
    all = BME280_ALL
};

components operator|(const components& a, const components& b) {
    return components(uint8_t(a) | uint8_t(b));
}

template<class I2CT, class DelayT>
class bme280
    : public self_pointing<bme280<I2CT, DelayT>>
    , public non_copyable
    , DelayT {
public:
    bme280(twi_addr_t addr, I2CT i2c, DelayT delay)
        : m_i2c(std::move(i2c))
        , DelayT(std::move(delay)) {
        m_dev.dev_id = addr.addr;
        m_dev.intf = BME280_I2C_INTF;

        m_dev.delay_ms = [](uint32_t ms, void* user) {
            auto self = static_cast<bme280*>(user);
            static_cast<DelayT&> (*self)(std::chrono::milliseconds{ms});
        };

        m_dev.read = [](uint8_t dev_id,
                        uint8_t reg_addr,
                        uint8_t* reg_data,
                        uint16_t len,
                        void* user) -> int8_t {
            auto self = static_cast<bme280*>(user);
            const char wb[] = {reg_addr};
            auto t = self->m_i2c->transmit({dev_id}, wb);
            if (t != twi_tx_res::ok)
                return 1;
            auto r = self->m_i2c->receive(
                {dev_id}, tos::span<char>(reinterpret_cast<char*>(reg_data), len));
            return r != twi_rx_res::ok;
        };

        m_dev.write = [](uint8_t dev_id,
                         uint8_t reg_addr,
                         uint8_t* reg_data,
                         uint16_t len,
                         void* user) -> int8_t {
            auto self = static_cast<bme280*>(user);
            char wb[32] = {reg_addr};
            if (len > std::size(wb) - 1)
                return 1;

            std::copy(reg_data, reg_data + len, wb + 1);
            auto r = self->m_i2c->transmit({dev_id}, wb);

            return r != twi_tx_res::ok;
        };

        m_dev.user = this;
        bme280_init(&m_dev);
    }

    bme280(bme280&& rhs)
        : m_dev{rhs.m_dev}
        , m_i2c{std::move(rhs.m_i2c)} {
        m_dev.user = this;
    }

    void set_config() {
        uint8_t settings_sel{};

        m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
        m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
        m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
        m_dev.settings.filter = BME280_FILTER_COEFF_16;
        m_dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

        settings_sel = BME280_OSR_PRESS_SEL;
        settings_sel |= BME280_OSR_TEMP_SEL;
        settings_sel |= BME280_OSR_HUM_SEL;
        settings_sel |= BME280_STANDBY_SEL;
        settings_sel |= BME280_FILTER_SEL;

        auto rslt = bme280_set_sensor_settings(settings_sel, &m_dev);
    }

    void sleep() {
        auto rslt = bme280_set_sensor_mode(BME280_SLEEP_MODE, &m_dev);
    }

    void enable() {
        auto rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &m_dev);
    }

    tos::expected<bme280_data, read_errors> read(components what = components::all) {
        bme280_data res;
        auto rslt = bme280_get_sensor_data(uint8_t(what), &res, &m_dev);
        if (rslt != BME280_OK) {
            return unexpected(read_errors(rslt));
        }
        return res;
    }

private:
    I2CT m_i2c;
    bme280_dev m_dev;
};
} // namespace bme280
} // namespace tos
