//
// Created by fatih on 6/14/18.
//

#pragma once

#include <stdint.h>
#include <tos/span.hpp>
#include <tos/devices.hpp>

namespace tos
{
    enum class twi_tx_res
    {
        ok = 0,
        addr_nack = 1,
        data_nack = 2,
        other = 3
    };

    enum class twi_rx_res
    {
        ok,
        addr_nack,
        data_nack,
        other
    };

    struct twim_data_rate
    {

    };

    struct twi_addr_t
    {
        uint8_t addr;
    };

    /**
     * Scans the I2C bus for a device with the given address.
     * This is implemented by transmitting an empty message to the given address
     * and seeing if we get an address NACK or not.
     * @param dev I2C bus driver to run on
     * @param addr address to scan
     * @return whether the device with the given address exists on the bus or not
     */
    template <class I2C>
    bool scan_address(I2C& dev, twi_addr_t addr)
    {
        auto res = dev->transmit(addr, empty_span<char>());
        return res == twi_tx_res::ok;
    }

    namespace i2c_type
    {
        struct slave_t {};
        struct master_t {};

        inline constexpr slave_t slave{};
        inline constexpr master_t master{};
    }

    namespace devs
    {
        template <int N> using i2c_t = dev<struct _i2c_t, N>;
        template <int N> static constexpr i2c_t<N> i2c{};
    } // namespace devs
}
