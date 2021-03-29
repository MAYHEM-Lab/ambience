#include <arch/i2c.hpp>
#include <tos_i2c_generated.hpp>
#include <i2c_generated.hpp>
#include <tos/task.hpp>

namespace {
using namespace tos::stm32;

struct impl final : tos::ae::service::i2c {
    tos::twi_rx_res receive(uint8_t addr, tos::span<uint8_t> data) override {
        return m_driv->receive({addr}, data);
    }

    tos::twi_tx_res transmit(uint8_t addr, tos::span<uint8_t> data) override {
        return m_driv->transmit({addr}, data);
    }

    tos::stm32::i2c* m_driv = nullptr;
};
} // namespace

tos::Task<tos::ae::service::i2c*> init_stm32_i2c() {
    co_return new impl{};
}