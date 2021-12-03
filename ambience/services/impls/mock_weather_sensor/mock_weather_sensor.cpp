#include <random_generated.hpp>
#include <weather_sensor_generated.hpp>

namespace tos::ae::services {
namespace {
struct null_sensor : weather_sensor::async_server {
    Task<weather_sample> sample() override {
        co_return weather_sample{0, 0, 0};
    }
};
} // namespace
} // namespace tos::ae::services

tos::Task<tos::ae::services::weather_sensor::async_server*> init_null_weather_sensor() {
    co_return new tos::ae::services::null_sensor;
}