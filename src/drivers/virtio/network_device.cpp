#include "common/inet/tcp_ip.hpp"
#include "tos/x86_64/port.hpp"
#include <cstdint>
#include <cstring>
#include <tos/debug/log.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/arch.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::virtio {
namespace {
constexpr auto mac_addr_offset = 0x14;
constexpr auto status_offset = 0x1a;
constexpr auto mtu_offset = 0x1e;

enum header_flags : uint8_t
{
    NEEDS_CSUM = 1,
    DATA_VALID = 2,
    RSC_INFO = 4
};

enum gso_types : uint8_t
{
    NONE = 0,
    TCPV4 = 1,
    UDP = 3,
    TCPV6 = 4,
    ECN = 0x80
};

struct virtio_net_hdr {
    header_flags flags;
    gso_types gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
};
} // namespace


struct network_device::buffer {
    virtio_net_hdr* header;
    void* data;
};

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

    auto recv_mem = palloc->allocate(1, 1);

    auto mem = palloc->address_of(*recv_mem);
    auto op_res = tos::cur_arch::allocate_region(
        tos::cur_arch::get_current_translation_table(),
        {{uintptr_t(mem), ptrdiff_t(4096)}, tos::permissions::read_write},
        tos::user_accessible::no,
        nullptr);
    LOG(bool(op_res));

    auto res = tos::cur_arch::mark_resident(
        tos::cur_arch::get_current_translation_table(),
        {{uintptr_t(mem), ptrdiff_t(4096)}, tos::permissions::read_write},
        tos::memory_types::normal,
        mem);
        
    LOG(bool(res));

    buffer b;

    auto header = new virtio_net_hdr{};
    b.header = header;
    b.data = mem;

    queue_rx_buf(std::move(b));

    tos::platform::set_irq(pci_dev().irq_line(),
                           tos::mem_function_ref<&network_device::isr>(*this));
    tos::x86_64::pic::enable_irq(pci_dev().irq_line());

    auto bar_base = this->bar_base();
    auto status_port = x86_64::port(bar_base + status_port_offset);
    status_port.outb(0xf);
    LOG_TRACE("Device initialized");
    return true;
}

uint32_t network_device::negotiate(uint32_t features) {
    return mac_feature | status_feature | mtu_feature | checksum_feature | merge_rxbuf_feature;
}

network_device::~network_device() = default;

network_device::network_device(x86_64::pci::device&& pci_dev) : device(std::move(pci_dev)) {
}
void network_device::queue_rx_buf(buffer&& buf) {
    auto q = queue_at(0);

    // auto [header_idx, header_desc] = q.alloc();
    auto [body_idx, body_desc] = q.alloc();

    // header_desc->addr = reinterpret_cast<uintptr_t>(buf.header);
    // header_desc->len = sizeof *buf.header;
    // header_desc->flags = queue_flags(queue_flags::next | queue_flags::write);
    // header_desc->next = body_idx;

    memset(buf.data, 0, 4096);

    body_desc->addr = reinterpret_cast<uintptr_t>(buf.data);
    body_desc->len = 1526;
    body_desc->flags = queue_flags::write;
    body_desc->next = 0;

    LOG("Placing", buf.data);

    q.available_base->ring[q.available_base->index++] = body_idx;

    m_buffers.emplace_back(std::move(buf));

    auto notify_port = x86_64::port(bar_base() + notify_port_offset);
    notify_port.outw(0);
}
} // namespace tos::virtio