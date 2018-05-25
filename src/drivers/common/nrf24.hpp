//
// Created by fatih on 5/25/18.
//

#pragma once

#include <drivers/arch/avr/gpio.hpp>
#include <tos/span.hpp>
#include <drivers/arch/avr/spi.hpp>
#include <util/delay.h>
#include <tos/algorithm.hpp>
#include "spi.hpp"

namespace tos
{
    enum class nrf24_speeds
    {
        s_2_mbits,
        s_1_mbits,
        s_250_kbits
    };

    enum class nrf24_power
    {

    };

    namespace nrf24_mnemonics
    {
        enum mnemonics : uint16_t
        {
            read_reg = 0,
            write_reg = 0x20,
            activate = 0x50,
            R_RX_PL_WID =  0x60,
            R_RX_PAYLOAD = 0x61,
            W_TX_PAYLOAD = 0xA0,
            W_ACK_PAYLOAD= 0xA8,
            FLUSH_TX     = 0xE1,
            FLUSH_RX     = 0xE2,
            REUSE_TX_PL  = 0xE3,
            RF24_NOP = 0xFF
        };
    };

    class nrf24
    {
    public:

        struct channel_t { uint8_t channel; };
        struct reg_id_t { uint8_t reg; };

        nrf24(pin_t ce, pin_t cs, pin_t interrupt);

        bool set_speed(nrf24_speeds speed);

        bool set_retries(uint8_t delay, uint8_t count);

        void enable_dyn_payloads();
        void disable_dyn_payloads();

        void set_channel(channel_t channel);
        channel_t get_channel() const;

        bool is_connected() const;

        void power_down();
        void power_up();

    private:
        uint8_t read_reg(reg_id_t reg, span<char> buf) const;
        uint8_t read_reg(reg_id_t reg) const;

        uint8_t write_reg(reg_id_t reg, uint8_t val);

        uint8_t write_cmd(nrf24_mnemonics::mnemonics cmd);

        spi_transaction<tos::avr::spi0> begin_transaction() const;

        void flush_rx();
        void flush_tx();

        pin_t m_cs_pin;
        pin_t m_ce_pin;
        pin_t m_int_pin;
    };

    namespace regs
    {
        static constexpr nrf24::reg_id_t config{0x00};
        static constexpr nrf24::reg_id_t setup_aw{0x03};
        static constexpr nrf24::reg_id_t setup_retr{0x04};
        static constexpr nrf24::reg_id_t rf_ch{0x05};
        static constexpr nrf24::reg_id_t setup{0x06};
        static constexpr nrf24::reg_id_t nrf_status{0x07};
        static constexpr nrf24::reg_id_t feature{0x1D};
        static constexpr nrf24::reg_id_t dynpd{0x1C};
    }
}

namespace tos
{
    namespace bits
    {
        static constexpr auto RF_DR_HIGH = 3;
        static constexpr auto RF_DR_LOW = 5;

        static constexpr auto ARD = 4;
        static constexpr auto ARC = 0;

        static constexpr auto enable_dyn_pay = 2;
        static constexpr auto enable_ack_pay = 1;
        static constexpr auto enable_dyn_ack = 0;

        template <uint8_t N>
        static constexpr auto DPL_P = N;

        static constexpr auto RX_DR = 6;
        static constexpr auto TX_DS = 5;
        static constexpr auto MAX_RT = 4;

        static constexpr auto PWR_UP = 1;

        static constexpr auto PRIM_RX = 0;
    }

    static tos::avr::gpio g;
    inline nrf24::nrf24(pin_t ce, pin_t cs, pin_t interrupt)
            : m_cs_pin{cs}, m_ce_pin{ce}, m_int_pin{interrupt} {
        g.set_pin_mode(ce, tos::pin_mode_t::out);
        g.write(ce, false);

        // TODO: use an alarm here
        _delay_ms(5);

        write_reg({ tos::regs::config }, 0x0C);

        disable_dyn_payloads();

        using namespace bits;
        write_reg( tos::regs::nrf_status, 1 << RX_DR | 1 << TX_DS | 1 << MAX_RT);

        set_channel({ 76 });

        flush_rx();
        flush_tx();

        write_reg(tos::regs::config, read_reg(regs::config) & ~(1 << PRIM_RX));
    }

    inline bool nrf24::set_speed(nrf24_speeds speed) {
        using namespace bits;

        auto setup_reg = read_reg(regs::setup);
        setup_reg &= ~((1 << RF_DR_HIGH) | (1 << RF_DR_LOW)); // 1 mbit

        if (speed == nrf24_speeds::s_250_kbits)
        {
            setup_reg |= (1 << RF_DR_LOW);
        }
        else if (speed == nrf24_speeds::s_2_mbits)
        {
            setup_reg |= (1 << RF_DR_HIGH);
        }

        write_reg(regs::setup, setup_reg);

        return read_reg(regs::setup) == setup_reg;
    }

    inline bool nrf24::set_retries(uint8_t delay, uint8_t count) {
        using namespace bits;

        auto reg = (delay & 0xf) << ARD | (count & 0xf) << ARC;
        write_reg(regs::setup_retr, reg);
        return read_reg(regs::setup_retr) == reg;
    }

    inline void nrf24::enable_dyn_payloads() {
        using namespace bits;

        write_reg(regs::feature, read_reg(regs::feature) | (1 << enable_dyn_pay));
        write_reg(regs::dynpd, read_reg(regs::dynpd) | 1 << DPL_P<5> | 1 << DPL_P<4> | 1 << DPL_P<3> |
                                                       1 << DPL_P<2> | 1 << DPL_P<1> | 1 << DPL_P<0>);
    }

    inline void nrf24::disable_dyn_payloads() {
        write_reg(regs::feature, 0);
        write_reg(regs::dynpd, 0);
    }

    inline void nrf24::set_channel(nrf24::channel_t channel) {
        constexpr uint8_t max_channel = 125;
        write_reg(regs::rf_ch, tos::min(channel.channel, max_channel));
    }

    inline nrf24::channel_t nrf24::get_channel() const {
        return { read_reg(regs::rf_ch) };
    }

    inline bool nrf24::is_connected() const {
        uint8_t setup = read_reg(regs::setup_aw);
        return setup >= 1 && setup <= 3;
    }

    inline void nrf24::power_down() {
        using namespace bits;
        g.write(m_ce_pin, false);
        write_reg(regs::config, read_reg(regs::config) & ~(1 << PWR_UP));
    }

    inline void nrf24::power_up() {
        using namespace bits;
        uint8_t cfg = read_reg(regs::config);

        if (!(cfg & (1 << PWR_UP)))
        {
            write_reg(regs::config, cfg | (1 << PWR_UP));

            // TODO: use an alarm here
            _delay_ms(5);
        }
    }

    inline uint8_t nrf24::write_cmd(nrf24_mnemonics::mnemonics cmd) {
        return begin_transaction().exchange(cmd);
    }

    inline void nrf24::flush_rx() {
        write_cmd(nrf24_mnemonics::FLUSH_RX);
    }

    inline void nrf24::flush_tx() {
        write_cmd(nrf24_mnemonics::FLUSH_TX);
    }
}