//
// Created by fatih on 12/6/18.
//

#include <common/lcd.hpp>
#include <tos/ft.hpp>
#include <arch/stm32/drivers.hpp>

#include <libopencm3/stm32/i2c.h>
#include <tos/version.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

namespace tos
{
    void delay_ms(std::chrono::milliseconds ms) {
        delay_us(ms);
    }

    void delay_us(std::chrono::microseconds us)
    {
        uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
        for (volatile int i = 0; i < end; ++i)
        {
            __asm__ __volatile__ ("nop");
        }
    }
} // namespace tos

namespace tos::stm32
{
    namespace detail
    {
        struct i2c_def
        {
            uint32_t i2c;
            rcc_periph_clken rcc;
        };

        static constexpr i2c_def i2cs[] = {
                { I2C1, RCC_I2C1 },
                { I2C2, RCC_I2C2 }
        };
    } // namespace detail

    class twim
            : public self_pointing<twim>
            , public tracked_driver<twim, 2>
    {
    public:
        twim(gpio::pin_type clk, gpio::pin_type data)
            : tracked_driver(0)
        {
            m_def = detail::i2cs + 0;

            rcc_periph_clock_enable(m_def->rcc);

            i2c_reset(m_def->i2c);

            rcc_periph_clock_enable(data.port->rcc);
            rcc_periph_clock_enable(clk.port->rcc);

            gpio_set_mode(data.port->which, GPIO_MODE_OUTPUT_50_MHZ,
                          GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, data.pin);

            gpio_set_mode(clk.port->which, GPIO_MODE_OUTPUT_50_MHZ,
                          GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, clk.pin);

            i2c_peripheral_disable(m_def->i2c);
            i2c_set_clock_frequency(m_def->i2c, I2C_CR2_FREQ_36MHZ);
            i2c_set_ccr(m_def->i2c, 0x1e);
            i2c_set_trise(m_def->i2c, 0x0b);
            i2c_set_own_7bit_slave_address(m_def->i2c, 0xab);
            i2c_peripheral_enable(m_def->i2c);
        }

        twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
        twi_tx_res receive(twi_addr_t from, span<char> buf) noexcept;

    private:

        const detail::i2c_def* m_def;
    };
} // namespace tos::stm32

void i2c1_ev_isr()
{
}


void i2c1_er_isr()
{

}

tos::twi_tx_res tos::stm32::twim::transmit(tos::twi_addr_t to, tos::span<const char> buf) noexcept {
    i2c_send_start(m_def->i2c);

    while (!((I2C_SR1(m_def->i2c) & I2C_SR1_SB)
             & (I2C_SR2(m_def->i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY))));

    i2c_send_7bit_address(m_def->i2c, to.addr, I2C_WRITE);

    while (!(I2C_SR1(m_def->i2c) & I2C_SR1_ADDR));

    auto reg32 = I2C_SR2(m_def->i2c);

    for (auto c : buf)
    {
        i2c_send_data(m_def->i2c, c);
        while (!(I2C_SR1(m_def->i2c) & I2C_SR1_BTF));
    }
    while (!(I2C_SR1(m_def->i2c) & (I2C_SR1_BTF | I2C_SR1_TxE)));

    i2c_send_stop(m_def->i2c);

    return tos::twi_tx_res::ok;
}

void lcd_main(void*)
{
    using namespace tos::tos_literals;
    using namespace tos; using namespace tos::stm32;

    // need a proper API for this alternate function IO business ...
    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;
    twim t { 24_pin, 25_pin };

    lcd<twim> lcd{ t, { 0x27 }, 20, 4 };

    auto tmr = open(devs::timer<2>);
    auto alarm = open(devs::alarm, tmr);

    lcd.begin(alarm);
    lcd.backlight();

    int x = 0;
    while (true)
    {
        ++x;
        using namespace std::chrono_literals;

        lcd.set_cursor(0, 0);
        tos::print(lcd, "Tos on STM32");

        lcd.set_cursor(0, 1);
        tos::print(lcd, tos::platform::arch_name, tos::platform::vendor_name);

        lcd.set_cursor(0, 2);
        tos::print(lcd, tos::span<const char>(tos::vcs::commit_hash).slice(0, 7));

        lcd.set_cursor(0, 3);
        tos::print(lcd, x);

        alarm.sleep_for(100ms);
    }
}

void tos_main()
{
    tos::launch(lcd_main);
}