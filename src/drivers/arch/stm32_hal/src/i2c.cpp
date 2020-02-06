//
// Created by fatih on 10/3/19.
//

#include <arch/i2c.hpp>

extern "C" {
#if defined(STM32F1) || defined(STM32L4) || defined(STM32F7)
void I2C1_EV_IRQHandler() {
    auto instance = tos::stm32::i2c::get(0);
    HAL_I2C_EV_IRQHandler(instance->native_handle());
}
void I2C1_ER_IRQHandler() {
    auto instance = tos::stm32::i2c::get(0);
    HAL_I2C_ER_IRQHandler(instance->native_handle());
}
void I2C2_EV_IRQHandler() {
    auto instance = tos::stm32::i2c::get(1);
    HAL_I2C_EV_IRQHandler(instance->native_handle());
}
void I2C2_ER_IRQHandler() {
    auto instance = tos::stm32::i2c::get(1);
    HAL_I2C_ER_IRQHandler(instance->native_handle());
}
#else
void I2C1_IRQHandler() {
    auto instance = tos::stm32::i2c::get(0);
    HAL_I2C_EV_IRQHandler(instance->native_handle());
    HAL_I2C_ER_IRQHandler(instance->native_handle());
}
void I2C2_IRQHandler() {
    auto instance = tos::stm32::i2c::get(1);
    HAL_I2C_EV_IRQHandler(instance->native_handle());
    HAL_I2C_ER_IRQHandler(instance->native_handle());
}
#endif

uint8_t get_i2c_id(I2C_HandleTypeDef* i2c) {
    switch (uintptr_t(i2c->Instance)) {
    case I2C1_BASE:
        return 0;
    case I2C2_BASE:
        return 1;
#if defined(I2C3)
    case I2C3_BASE:
        return 2;
#endif
    }
    TOS_UNREACHABLE();
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    auto instance = tos::stm32::i2c::get(get_i2c_id(hi2c));
    instance->tx_fin();
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    auto instance = tos::stm32::i2c::get(get_i2c_id(hi2c));
    instance->rx_fin();
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c) {
    auto instance = tos::stm32::i2c::get(get_i2c_id(hi2c));
    instance->err();
}
}

namespace tos::stm32 {
namespace {
void open_scl(gpio::pin_type pin) {
    enable_rcc(pin.port);
    GPIO_InitTypeDef init;
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_AF_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = detail::gpio_speed::highest();
    HAL_GPIO_Init(pin.port, &init);
}

void open_sda(gpio::pin_type pin) {
    open_scl(pin); // it's the same thing
}

#if defined(STM32L0) || defined(STM32L4)
uint32_t make_timing_register_100khz() {
    // The values are taken from the reference manual, I2C Timing settings section (Table
    // 104).

    uint32_t presc = 3;
    uint32_t scll = 0x13;
    uint32_t sclh = 0xF;
    uint32_t sdadel = 2;
    uint32_t scldel = 4;

    return presc << I2C_TIMINGR_PRESC_Pos | scldel << I2C_TIMINGR_SCLDEL_Pos |
           sdadel << I2C_TIMINGR_SDADEL_Pos | sclh << I2C_TIMINGR_SCLH_Pos | scll;
}
#endif
} // namespace

i2c::i2c(const detail::i2c_def& def, gpio::pin_type scl, gpio::pin_type sda)
    : tracked_driver(std::distance(&detail::i2cs[0], &def))
    , m_handle{} {
#if defined(STM32F1)
    if (def.instance == I2C1) {
        // Remapping
        if (scl.pin == GPIO_PIN_8) {
            __HAL_RCC_AFIO_CLK_ENABLE();
            __HAL_AFIO_REMAP_I2C1_ENABLE();
        } else {
            __HAL_RCC_AFIO_CLK_ENABLE();
            __HAL_AFIO_REMAP_I2C1_DISABLE();
        }
    }
#endif

    open_scl(scl);
    open_sda(sda);

    def.rcc();

    m_handle.Instance = def.instance;
    I2C_InitTypeDef& init = m_handle.Init;
    init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    init.OwnAddress1 = 0;
    init.OwnAddress2 = 0;
#if defined(STM32F1)
    init.ClockSpeed = 100'000;
    init.DutyCycle = I2C_DUTYCYCLE_2;
#elif defined(STM32L0) || defined(STM32L4)
    init.Timing = make_timing_register_100khz();
#endif

    if (HAL_I2C_Init(&m_handle) != HAL_OK) {
        tos::debug::panic("Can't initialize STM32 I2C Driver");
    }

    HAL_NVIC_SetPriority(def.ev_irq, 0, 0);
    HAL_NVIC_EnableIRQ(def.ev_irq);

#if !defined(STM32L0)
    HAL_NVIC_SetPriority(def.er_irq, 0, 0);
    HAL_NVIC_EnableIRQ(def.er_irq);
#endif
}

twi_tx_res i2c::transmit(twi_addr_t to, span<const uint8_t> buf) noexcept {
    auto uint8_span = raw_cast<const uint8_t>(buf);
    auto transmit_res =
        HAL_I2C_Master_Transmit_IT(native_handle(),
                                   to.addr << 1,
                                   const_cast<uint8_t*>(uint8_span.data()),
                                   uint8_span.size());
    if (transmit_res != HAL_OK) {
        return twi_tx_res::other;
    }

    tos::kern::busy();
    m_wait.down();
    tos::kern::unbusy();

    switch (native_handle()->ErrorCode)
    {
        case HAL_I2C_ERROR_AF:
            return twi_tx_res::addr_nack;
        case HAL_I2C_ERROR_NONE:
            return twi_tx_res::ok;
        default:
            return twi_tx_res::other;
    }
}

twi_rx_res i2c::receive(twi_addr_t from, span<uint8_t> buf) noexcept {
    auto uint8_span = raw_cast<uint8_t>(buf);
    auto transmit_res =
        HAL_I2C_Master_Receive_IT(native_handle(),
                                   from.addr << 1,
                                   uint8_span.data(),
                                   uint8_span.size());
    if (transmit_res != HAL_OK) {
        return twi_rx_res::other;
    }

    tos::kern::busy();
    m_wait.down();
    tos::kern::unbusy();

    switch (native_handle()->ErrorCode)
    {
        case HAL_I2C_ERROR_AF:
            return twi_rx_res::addr_nack;
        case HAL_I2C_ERROR_NONE:
            return twi_rx_res::ok;
        default:
            return twi_rx_res::other;
    }
}

void i2c::tx_fin() {
    m_wait.up_isr();
}

void i2c::rx_fin() {
    m_wait.up_isr();
}

void i2c::err() {
    m_wait.up_isr();
}
} // namespace tos::stm32