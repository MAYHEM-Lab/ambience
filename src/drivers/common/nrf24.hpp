//
// Created by fatih on 5/25/18.
//

#pragma once

#include <drivers/arch/avr/drivers.hpp>
#include <tos/span.hpp>
#include <util/delay.h>
#include <tos/algorithm.hpp>
#include <algorithm>
#include "spi.hpp"
#include <avr/pgmspace.h>

namespace tos
{
    enum class nrf24_speeds : uint8_t
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
    }

    enum class nrf24_addr_width : uint8_t
    {
        w_3_bytes = 0b01,
        w_4_bytes = 0b10,
        w_5_bytes = 0b11
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

        void set_power_level(nrf24_power);

        size_t get_next_length() const;

        void set_addr_width(nrf24_addr_width);
        nrf24_addr_width get_addr_width() const;

        void open_read_pipe(uint8_t num, span<const uint8_t> addr);
        void open_write_pipe(span<const uint8_t> addr);

        void start_listening();

    private:
        uint8_t read_reg(reg_id_t reg, span<uint8_t> buf) const;
        uint8_t read_reg(reg_id_t reg) const;

        uint8_t write_reg(reg_id_t reg, uint8_t val);
        uint8_t write_reg(reg_id_t reg, span<const uint8_t> val);

        uint8_t write_cmd(nrf24_mnemonics::mnemonics cmd);

        spi_transaction<tos::avr::spi0> begin_transaction() const;

        void enable_chip();
        void disable_chip();

        void flush_rx();
        void flush_tx();

        pin_t m_cs_pin;
        pin_t m_ce_pin;
        pin_t m_int_pin;
    };

    namespace regs
    {
        static constexpr nrf24::reg_id_t config{0x00};
        static constexpr nrf24::reg_id_t EN_RXADDR {0x02};
        static constexpr nrf24::reg_id_t setup_aw{0x03};
        static constexpr nrf24::reg_id_t setup_retr{0x04};
        static constexpr nrf24::reg_id_t rf_ch{0x05};
        static constexpr nrf24::reg_id_t setup{0x06};
        static constexpr nrf24::reg_id_t nrf_status{0x07};
        static constexpr nrf24::reg_id_t feature{0x1D};
        static constexpr nrf24::reg_id_t dynpd{0x1C};

        template <uint8_t N>
        static constexpr nrf24::reg_id_t RX_ADDR_P {0x0A + N};

        static constexpr nrf24::reg_id_t TX_ADDR {0x10};

        template <uint8_t N>
        static constexpr nrf24::reg_id_t RX_PW_P {0x11 + N};
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

        static constexpr auto EN_DPL = 2;
        static constexpr auto EN_ACK_PAY = 1;
        static constexpr auto EN_DYN_ACK = 0;

        template <uint8_t N>
        static constexpr auto DPL_P = N;

        static constexpr auto RX_DR = 6;
        static constexpr auto TX_DS = 5;
        static constexpr auto MAX_RT = 4;

        static constexpr auto PWR_UP = 1;

        static constexpr auto PRIM_RX = 0;

        template <uint8_t N>
        static constexpr auto ERX_P = N;
    }

    static tos::avr::gpio g;
    inline nrf24::nrf24(pin_t ce, pin_t cs, pin_t interrupt)
            : m_cs_pin{cs}, m_ce_pin{ce}, m_int_pin{interrupt} {
        g.set_pin_mode(ce, tos::pin_mode::out);
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

        write_reg(regs::feature, read_reg(regs::feature) | (1 << EN_ACK_PAY));
        write_reg(regs::dynpd, read_reg(regs::dynpd) | 1 << DPL_P<5> | 1 << DPL_P<4> | 1 << DPL_P<3> |
                                                       1 << DPL_P<2> | 1 << DPL_P<1> | 1 << DPL_P<0>);
    }

    inline void nrf24::disable_dyn_payloads() {
        write_reg(regs::feature, 0);
        write_reg(regs::dynpd, 0);
    }

    inline void nrf24::set_channel(nrf24::channel_t channel) {
        constexpr uint8_t max_channel = 125;
        write_reg(regs::rf_ch, std::min(channel.channel, max_channel));
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

    inline size_t nrf24::get_next_length() const {
        auto trans = begin_transaction();
        trans.exchange(nrf24_mnemonics::R_RX_PL_WID);
        return trans.exchange(0xff);
    }

    inline void nrf24::set_addr_width(nrf24_addr_width w) {
        write_reg(regs::setup_aw, static_cast<uint8_t>(w));
    }

    inline nrf24_addr_width nrf24::get_addr_width() const {
        return static_cast<nrf24_addr_width>(read_reg(regs::setup_aw) & 0b11);
    }


    inline void nrf24::open_read_pipe(uint8_t num, span<const uint8_t> addr)
    {
        using namespace regs;
        using namespace bits;

        static constexpr reg_id_t child_pipe[] PROGMEM =
        {
            RX_ADDR_P<0>, RX_ADDR_P<1>, RX_ADDR_P<2>, RX_ADDR_P<3>, RX_ADDR_P<4>, RX_ADDR_P<5>
        };

        static constexpr reg_id_t child_payload_size[] PROGMEM =
        {
            RX_PW_P<0>, RX_PW_P<1>, RX_PW_P<2>, RX_PW_P<3>, RX_PW_P<4>, RX_PW_P<5>
        };
        static constexpr uint8_t child_pipe_enable[] PROGMEM =
        {
            ERX_P<0>, ERX_P<1>, ERX_P<2>, ERX_P<3>, ERX_P<4>, ERX_P<5>
        };

        if (num >= 2)
        {
            write_reg(reg_id_t{pgm_read_byte(&child_pipe[num])}, addr[0]);
        }
        else
        {
            write_reg(reg_id_t{pgm_read_byte(&child_pipe[num])}, addr);
        }

        write_reg(reg_id_t{pgm_read_byte(&child_payload_size[num])}, 32);

        write_reg(EN_RXADDR, read_reg(EN_RXADDR) | (1 << (pgm_read_byte(&child_pipe_enable[num]))));
    }

    inline void nrf24::start_listening() {
        write_reg(regs::config, read_reg(regs::config) | (1 << bits::PRIM_RX));
        write_reg(regs::nrf_status, 1 << bits::RX_DR | 1 << bits::TX_DS | 1 << bits::MAX_RT);

        enable_chip();

        if (read_reg(regs::feature) & (1 << bits::EN_ACK_PAY))
        {
            flush_tx();
        }
    }

    inline void nrf24::enable_chip() {
        g.write(m_ce_pin, true);
    }

    inline void nrf24::disable_chip() {
        g.write(m_ce_pin, false);
    }
}