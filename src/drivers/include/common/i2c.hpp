//
// Created by fatih on 6/14/18.
//

#pragma once

#include <cstdint>
#include <nonstd/variant.hpp>
#include <tos/devices.hpp>
#include <tos/expected.hpp>
#include <tos/span.hpp>
#include <tos_i2c_generated.hpp>

namespace tos {
namespace i2c {
struct address7 {
    uint8_t addr;
};

using errors = mpark::variant<twi_tx_res, twi_rx_res>;

template<class I2C>
expected<uint8_t, errors> read_byte_data(I2C& i2c, address7 address, uint8_t command) {
    auto tx_res = i2c->transmit(address, monospan(command));
    if (tx_res != twi_tx_res::ok) {
        return unexpected(errors{tx_res});
    }
    uint8_t data;
    auto rx_res = i2c->receive(address, monospan(data));
    if (rx_res != twi_rx_res::ok) {
        return unexpected(errors{rx_res});
    }
    return data;
}

template<class I2C>
expected<void, errors>
write_byte_data(I2C& i2c, address7 address, uint8_t command, uint8_t data) {
    auto tx_res = i2c->transmit(address, monospan(command));
    if (tx_res != twi_tx_res::ok) {
        return unexpected(errors{tx_res});
    }
    tx_res = i2c->transmit(address, monospan(data));
    if (tx_res != twi_tx_res::ok) {
        return unexpected(errors{tx_res});
    }
    return {};
}
} // namespace i2c

struct twim_data_rate {};

using twi_addr_t = i2c::address7;

constexpr bool operator==(twi_addr_t left, twi_addr_t right) {
    return left.addr == right.addr;
}

/**
 * Scans the I2C bus for a device with the given address.
 * This is implemented by transmitting an empty message to the given address
 * and seeing if we get an address NACK or not.
 * @param dev I2C bus driver to run on
 * @param addr address to scan
 * @return whether the device with the given address exists on the bus or not
 */
template<class I2C>
bool scan_address(I2C& dev, twi_addr_t addr) {
    auto res = dev->transmit(addr, empty_span<char>());
    return res == twi_tx_res::ok;
}

namespace i2c_type {
struct slave_t {};
struct master_t {};

inline constexpr slave_t slave{};
inline constexpr master_t master{};
} // namespace i2c_type

namespace devs {
template<int N>
using i2c_t = dev<struct _i2c_t, N>;
template<int N>
static constexpr i2c_t<N> i2c{};
} // namespace devs

struct any_i2c {
public:
    virtual twi_tx_res transmit(i2c::address7, span<const uint8_t>) = 0;
    virtual twi_rx_res receive(i2c::address7, span<uint8_t>) = 0;
    virtual ~any_i2c() = 0;
};
} // namespace tos
