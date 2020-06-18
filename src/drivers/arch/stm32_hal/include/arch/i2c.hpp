//
// Created by fatih on 10/3/19.
//

#pragma once

#include "gpio.hpp"

#include <algorithm>
#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <stm32_hal/dma.hpp>
#include <stm32_hal/i2c.hpp>
#include <stm32_hal/rcc.hpp>
#include <tos/semaphore.hpp>

namespace tos::stm32 {
namespace detail {
#if !defined(STM32L0)
struct i2c_def {
    I2C_TypeDef* instance;
    IRQn_Type ev_irq;
    IRQn_Type er_irq;
    void (*rcc)();
};

inline const i2c_def i2cs[] = {
    {I2C1,
     I2C1_EV_IRQn,
     I2C1_ER_IRQn,
     [] {
         __HAL_RCC_I2C1_CLK_ENABLE();
         __HAL_RCC_I2C1_FORCE_RESET();
         HAL_Delay(10);
         __HAL_RCC_I2C1_RELEASE_RESET();
     }},
#if defined(I2C2)
    {I2C2,
     I2C2_EV_IRQn,
     I2C2_ER_IRQn,
     [] {
         __HAL_RCC_I2C2_CLK_ENABLE();
         __HAL_RCC_I2C2_FORCE_RESET();
         HAL_Delay(10);
         __HAL_RCC_I2C2_RELEASE_RESET();
     }},
#endif
#if defined(I2C3)
    {I2C3,
     I2C3_EV_IRQn,
     I2C3_ER_IRQn,
     [] {
         __HAL_RCC_I2C3_CLK_ENABLE();
         __HAL_RCC_I2C3_FORCE_RESET();
         HAL_Delay(10);
         __HAL_RCC_I2C3_RELEASE_RESET();
     }},
#endif
};
#else
struct i2c_def {
    I2C_TypeDef* instance;
    IRQn_Type ev_irq;
    void (*rcc)();
};

const i2c_def i2cs[] = {
    {I2C1,
     I2C1_IRQn,
     [] {
         __HAL_RCC_I2C1_CLK_ENABLE();
         __HAL_RCC_I2C1_FORCE_RESET();
         HAL_Delay(10);
         __HAL_RCC_I2C1_RELEASE_RESET();
     }},
    {I2C2,
     I2C2_IRQn,
     [] {
         __HAL_RCC_I2C2_CLK_ENABLE();
         __HAL_RCC_I2C2_FORCE_RESET();
         HAL_Delay(10);
         __HAL_RCC_I2C2_RELEASE_RESET();
     }},
};

#endif
} // namespace detail

class i2c
    : public self_pointing<i2c>
    , public tracked_driver<i2c, std::size(detail::i2cs)>
    , public non_copy_movable {
public:
    i2c(const detail::i2c_def& def, gpio::pin_type scl, gpio::pin_type sda);

    I2C_HandleTypeDef* native_handle() {
        return &m_handle;
    }

    twi_tx_res transmit(twi_addr_t to, span<const uint8_t> buf) noexcept;
    twi_rx_res receive(twi_addr_t from, span<uint8_t> buf) noexcept;

    void tx_fin();
    void rx_fin();
    void err();

private:
    I2C_HandleTypeDef m_handle;
    semaphore m_wait{0};
};
} // namespace tos::stm32
namespace tos {
inline stm32::i2c open_impl(devs::i2c_t<1>,
                           i2c_type::master_t,
                            stm32::gpio::pin_type scl,
                            stm32::gpio::pin_type sda) {
    return stm32::i2c{stm32::detail::i2cs[0], scl, sda};
}

inline stm32::i2c open_impl(devs::i2c_t<2>,
                            i2c_type::master_t,
                            stm32::gpio::pin_type scl,
                            stm32::gpio::pin_type sda) {
    return stm32::i2c{stm32::detail::i2cs[1], scl, sda};
}
}