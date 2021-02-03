#include "common/inet/tcp_ip.hpp"
#include "tos/x86_64/port.hpp"
#include <cstdint>
#include <tos/debug/log.hpp>
#include <tos/virtio/network_device.hpp>

namespace tos::virtio {
namespace {
constexpr auto mac_addr_offset = 0x14;
constexpr auto status_offset = 0x1a;
constexpr auto mtu_offset = 0x1c;
} // namespace

mac_addr_t network_device::address() const {
    auto bar_base = this->bar_base();
    mac_addr_t res;
    for (int i = 0; i < 6; ++i) {
        res.addr[i] = x86_64::port(bar_base + mac_addr_offset + i).inb();
    }
    return res;
}

size_t network_device::mtu() const {
    return x86_64::port(bar_base() + mtu_offset).inw();
}

bool network_device::initialize(physical_page_allocator* palloc) {
    if (!base_initialize(palloc)) {
        return false;
    }
    LOG("Status:", x86_64::port(bar_base() + status_offset).inw());
    return true;
}

uint32_t network_device::negotiate(uint32_t features) {
    return features & ~(ring_event_idx | control_queue_feature | multiqueue_feature) |
           mac_feature | status_feature | mtu_feature | checksum_feature;
}
} // namespace tos::virtio