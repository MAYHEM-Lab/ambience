#include <agent_generated.hpp>
#include <alarm_generated.hpp>
#include <weather_sensor_generated.hpp>
#include <tos/debug/log.hpp>

namespace tos::ae::services {
namespace {
struct sensor_sampler : agent::async_server {
    sensor_sampler(alarm::async_server* alarm, weather_sensor::async_server* sensor)
        : m_alarm(alarm)
        , m_sensor(sensor) {
    }

    tos::Task<bench_result> start(const int64_t& param) override {
        tos::debug::log("Starting sampler");

        while (true) {
            std::vector<weather_sample> local_samples;
            local_samples.reserve(8);
            for (int i = 0; i < 8; ++i) {
                auto sample = co_await m_sensor->sample();
                const auto [temp, humid, pressure] = sample;
                local_samples.push_back(sample);
                tos::debug::log(int(temp), int(humid), int(pressure));
                co_await m_alarm->sleep_for(milliseconds{100});
            }

            co_await m_alarm->sleep_for(milliseconds{15000});
        }
    }

    alarm::async_server* m_alarm;
    weather_sensor::async_server* m_sensor;
};
} // namespace
} // namespace tos::ae::services

tos::Task<tos::ae::agent::async_server*>
init_weather_sampler(tos::ae::services::alarm::async_server* alarm,
                     tos::ae::services::weather_sensor::async_server* sensor) {
    co_return new tos::ae::services::sensor_sampler(alarm, sensor);
}