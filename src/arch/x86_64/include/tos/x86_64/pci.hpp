#pragma once

#include <array>
#include <common/pci/classes.hpp>
#include <cstdint>
#include <optional>
#include <tos/x86_64/port.hpp>

namespace tos::x86_64::pci {
namespace detail {
inline constexpr port address_port{0xcf8};
inline constexpr port data_port{0xcfc};

inline uint32_t config_read_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto lbus = static_cast<uint32_t>(bus);
    auto lslot = static_cast<uint32_t>(slot);
    auto lfunc = static_cast<uint32_t>(func);

    auto address = lbus << 16 | lslot << 11 | lfunc << 8 | (offset & 0xfc) | 0x80000000;
    address_port.outl(address);

    auto data = data_port.inl();
    return data;
}

inline void config_write_raw(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    auto lbus = static_cast<uint32_t>(bus);
    auto lslot = static_cast<uint32_t>(slot);
    auto lfunc = static_cast<uint32_t>(func);

    auto address = lbus << 16 | lslot << 11 | lfunc << 8 | (offset & 0xfc) | 0x80000000;
    address_port.outl(address);

    data_port.outl(value);
}

inline uint32_t
config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    return data;
}

inline uint16_t
config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b10;
    auto bits = in_offset * 8;

    return (data >> bits) & 0xffff;
}

inline uint8_t config_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b11;
    auto bits = in_offset * 8;

    return (data >> bits) & 0xff;
}

inline void config_write_dword(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    return config_write_raw(bus, slot, func, offset, value);
}

inline void config_write_byte(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value) {
    auto qword = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b11;
    auto bits = in_offset * 8;

    qword &= ~(0xff << bits);
    qword |= static_cast<uint32_t>(value) << bits;

    config_write_raw(bus, slot, func, offset, qword);
}
} // namespace detail

inline uint16_t get_vendor(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 0);
}

inline uint16_t get_dev_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 2);
}

inline uint8_t get_class_code(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xa + 1);
}

inline uint8_t get_subclass(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xa);
}

inline uint16_t get_subsys_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 0x2c + 2);
}

inline uint8_t get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xc + 2);
}
} // namespace tos::x86_64::pci

namespace tos::x86_64::pci {
class device;
struct capability {

    uint8_t vendor() const;
    std::optional<capability> next() const;
    uint8_t len() const;

    uint8_t read_byte(uint8_t offset) const;
    uint16_t read_word(uint8_t offset) const;
    uint32_t read_long(uint8_t offset) const;

    const device* m_dev;
    uint8_t m_offset;
};

class device {
public:
    device(uint8_t bus, uint8_t slot, uint8_t func)
        : m_bus{bus}
        , m_slot{slot}
        , m_func{func} {
    }

    tos::pci::classes class_code() const {
        return tos::pci::classes(get_class_code(m_bus, m_slot, m_func));
    }

    bool has_capabilities() const {
        return status() & 1 << 4;
    }

    uint16_t vendor() const {
        return get_vendor(m_bus, m_slot, m_func);
    }

    uint16_t device_id() const {
        return get_dev_id(m_bus, m_slot, m_func);
    }

    uint16_t status() const {
        return detail::config_read_word(m_bus, m_slot, m_func, 0x6);
    }

    uint32_t bar0() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x10);
    }

    uint32_t bar1() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x14);
    }

    uint32_t bar2() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x18);
    }

    uint32_t bar3() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x1C);
    }

    uint32_t bar4() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x20);
    }

    uint32_t bar5() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x24);
    }

    std::array<uint32_t, 6> bars() const {
        return {bar0(), bar1(), bar2(), bar3(), bar4(), bar5()};
    }

    std::optional<capability> capabilities_root() const {
        if (has_capabilities()) {
            return capability{
                this,
                static_cast<uint8_t>(
                    detail::config_read_byte(m_bus, m_slot, m_func, 0x34) & 0xFC)};
        }
        return {};
    }

    uint8_t irq_line() const {
        return detail::config_read_byte(m_bus, m_slot, m_func, 0x3c);
    }

    void irq_line(uint8_t line) {
        detail::config_write_byte(m_bus, m_slot, m_func, 0x3c, line);
    }

    template<class Fn, class... Args>
    auto call_fn(const Fn& fn, Args&&... args) const {
        return std::invoke(fn, m_bus, m_slot, m_func, std::forward<Args>(args)...);
    }

private:
    uint8_t m_bus, m_slot, m_func;
};

inline uint8_t capability::vendor() const {
    return m_dev->call_fn(detail::config_read_byte, m_offset);
}

inline std::optional<capability> capability::next() const {
    auto ptr = m_dev->call_fn(detail::config_read_byte, m_offset + 1);
    if (ptr != 0) {
        return capability{m_dev, ptr};
    }
    return std::optional<capability>();
}

inline uint8_t capability::len() const {
    return m_dev->call_fn(detail::config_read_byte, m_offset + 2);
}

inline uint8_t capability::read_byte(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_byte, m_offset + offset);
}
inline uint16_t capability::read_word(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_word, m_offset + offset);
}
inline uint32_t capability::read_long(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_dword, m_offset + offset);
}
} // namespace tos::x86_64::pci
