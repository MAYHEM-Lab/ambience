#include <tos_i2c_generated.hpp>
#include <i2c_generated.hpp>
#include <temp_sensor_generated.hpp>
#include <tos/device/bme280.hpp>

namespace {
auto delay = [](std::chrono::microseconds) {};
using driv = tos::device::bme280::driver<tos::ae::services::i2c::sync_server* const, decltype(delay)>;
struct impl final : tos::ae::services::temp_sensor::sync_server {
    impl(tos::ae::service::i2c::sync_server& i2c) : m_driver{{0x1}, &i2c, delay} {}

    float sample() override {
        return force_get(m_driver->read()).temperature;
    }

    driv m_driver;
};
} // namespace

extern "C" tos::ae::service::temp_sensor::sync_server* init_bme280(tos::ae::services::i2c::sync_server& i2c) {
    return new impl{i2c};
}