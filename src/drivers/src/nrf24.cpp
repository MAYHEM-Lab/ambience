//
// Created by fatih on 5/25/18.
//

#include <drivers/common/nrf24.hpp>

namespace tos
{
    spi_transaction<tos::avr::spi0> nrf24::begin_transaction() const {
        return spi_transaction<tos::avr::spi0>{m_cs_pin};
    }

    static constexpr auto reg_mask = 0x1F;

    uint8_t nrf24::write_reg(nrf24::reg_id_t reg, uint8_t val) {
        auto trans = begin_transaction();
        auto res = trans.exchange(nrf24_mnemonics::write_reg | (reg_mask & reg.reg));
        trans.exchange(val);
        return res;
    }

    uint8_t nrf24::write_reg(nrf24::reg_id_t reg, span<const uint8_t> val) {
        auto trans = begin_transaction();
        auto res = trans.exchange(nrf24_mnemonics::write_reg | (reg_mask & reg.reg));
        for (auto byte : val)
        {
            trans.exchange(byte);
        }
        return res;
    }

    uint8_t nrf24::read_reg(nrf24::reg_id_t reg, span<uint8_t> b) const {
        auto trans = begin_transaction();
        auto res = trans.exchange(nrf24_mnemonics::read_reg | (reg_mask & reg.reg));
        for (auto& byte : b)
        {
            byte = trans.exchange(0xff);
        }
        return res;
    }

    uint8_t nrf24::read_reg(nrf24::reg_id_t reg) const {
        auto trans = begin_transaction();
        trans.exchange(nrf24_mnemonics::read_reg | (reg_mask & reg.reg));
        return trans.exchange(0xff);
    }
}
