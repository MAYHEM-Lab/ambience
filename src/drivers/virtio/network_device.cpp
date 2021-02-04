#include "common/inet/tcp_ip.hpp"
#include "tos/intrusive_list.hpp"
#include "tos/semaphore.hpp"
#include "tos/x86_64/port.hpp"
#include <cstdint>
#include <cstring>
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pic.hpp>

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

struct buf {
    list_node<buf> list_node;
    virtio_net_hdr header;
    uint8_t mem[];

    span<uint8_t> body() {
        return span<uint8_t>(mem, header.hdr_len);
    }
};

struct [[gnu::packed]] tx_header {
    tos::semaphore sem{0};
    virtio_net_hdr header;
};

intrusive_list<buf, through_member<&buf::list_node>> received_packets;
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

    auto buf_ptr = new (mem) buf{};

    queue_rx_buf(*buf_ptr);

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
    return mac_feature | status_feature | mtu_feature | checksum_feature |
           merge_rxbuf_feature;
}

network_device::~network_device() = default;

network_device::network_device(x86_64::pci::device&& pci_dev)
    : device(std::move(pci_dev)) {
}

void network_device::queue_rx_buf(buf& buffer) {
    auto rx_queue = queue_at(0);

    auto [body_idx, body_desc] = rx_queue.alloc();

    body_desc->addr = reinterpret_cast<uintptr_t>(&buffer.header);
    body_desc->len = sizeof(virtio_net_hdr) + mtu();
    body_desc->flags = queue_flags::write;
    body_desc->next = 0;

    rx_queue.submit_available(body_idx);

    auto notify_port = x86_64::port(bar_base() + notify_port_offset);
    notify_port.outw(0);
}

void network_device::isr(tos::x86_64::exception_frame* f, int num) {
    asm volatile("push %rax");
    auto bar_base = this->bar_base();
    auto isr_status_port = x86_64::port(bar_base + interrupt_status_port_offset);
    auto isr_status = isr_status_port.inb();
    LOG("ISR:", int(isr_status));
    if (isr_status & 1) {
        auto& rx_queue = queue_at(0);
        auto& tx_queue = queue_at(1);

        rx_queue.for_each_used([&](queue_used_elem el) {
            LOG("New used elem");

            auto total_len = el.len;

            // The received packet consists of 2 descriptors: header + body
            auto& body_desc = rx_queue.descriptors()[el.id];

            auto buf_ptr = reinterpret_cast<buf*>(
                reinterpret_cast<char*>(body_desc.addr) - offsetof(buf, header));

            LOG((void*)buf_ptr->header.flags,
                buf_ptr->header.num_buffers,
                buf_ptr->header.hdr_len);
            buf_ptr->header.hdr_len = total_len - sizeof buf_ptr->header;
            LOG(buf_ptr->body());

            received_packets.push_back(*buf_ptr);
            m_rx_sem.up_isr();
        });

        tx_queue.for_each_used([&](queue_used_elem el) {
            LOG("Transmitted!");
            auto& desc = tx_queue.descriptors()[el.id];
            auto header_ptr = reinterpret_cast<tx_header*>(
                reinterpret_cast<char*>(desc.addr) - offsetof(tx_header, header));
            header_ptr->sem.up_isr();
        });
    }
    asm volatile("pop %rax");
}

span<uint8_t> network_device::take_packet() {
    m_rx_sem.down();
    auto& b = received_packets.front();
    LOG("Giving packet at", &b);
    received_packets.pop_front();
    return b.body();
}

void network_device::return_packet(span<uint8_t> buffer) {
    auto b = reinterpret_cast<buf*>(buffer.data() - offsetof(buf, mem));
    Assert(b->header.hdr_len == buffer.size());
    LOG("Getting packet at", b);
    queue_rx_buf(*b);
}

void network_device::transmit_packet(span<const uint8_t> data) {
    auto& tx_queue = queue_at(1);

    auto [header_idx, header_desc] = tx_queue.alloc();
    auto [body_idx, body_desc] = tx_queue.alloc();

    tx_header header{};

    header_desc->addr = reinterpret_cast<uintptr_t>(&header.header);
    header_desc->len = sizeof header.header;
    header_desc->flags = queue_flags(queue_flags::next);
    header_desc->next = body_idx;

    body_desc->addr = reinterpret_cast<uintptr_t>(data.data());
    body_desc->len = data.size();
    body_desc->flags = queue_flags::none;
    body_desc->next = 0;

    tx_queue.submit_available(header_idx);
    auto notify_port = x86_64::port(bar_base() + notify_port_offset);
    notify_port.outw(1);

    header.sem.down();
}
} // namespace tos::virtio