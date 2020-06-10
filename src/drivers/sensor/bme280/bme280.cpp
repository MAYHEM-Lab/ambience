#include <tos/device/bme280.hpp>

namespace tos::device::bme280 {
namespace {
auto delay = [](std::chrono::milliseconds) {};
driver<any_i2c*, decltype(delay)> d({}, nullptr, delay);
}
}