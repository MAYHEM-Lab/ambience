#include "common/inet/tcp_ip.hpp"
#include "tos/intrusive_list.hpp"
#include "tos/memory.hpp"
#include "tos/semaphore.hpp"
#include <cstring>
#include <tos/debug/log.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::virtio {
namespace {
constexpr auto dev_base_offset = 0x14;
constexpr auto dev_mac_addr_offset = dev_base_offset + 0;
constexpr auto dev_status_offset = dev_base_offset + 6;
constexpr auto dev_mtu_offset = dev_base_offset + 10;

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

struct tx_header {
    tos::semaphore sem{0};
    virtio_net_hdr header;
};

intrusive_list<buf, through_member<&buf::list_node>> received_packets;
} // namespace

mac_addr_t network_device::address() const {
    mac_addr_t res;
    for (int i = 0; i < 6; ++i) {
        res.addr[i] =
            transport().read_byte((have_msix() ? 4 : 0) + dev_mac_addr_offset + i);
    }
    return res;
}

size_t network_device::mtu() const {
    return transport().read_u16((have_msix() ? 4 : 0) + dev_mtu_offset);
}

bool network_device::initialize(physical_page_allocator* palloc) {
    if (!base_initialize(palloc)) {
        return false;
    }
    LOG("Status:", transport().read_u16((have_msix() ? 4 : 0) + dev_status_offset));

    for (int i = 0; i < 200; ++i) {
        auto recv_mem = palloc->allocate(1, 1);

        auto mem = palloc->address_of(*recv_mem);
        auto map_res = tos::cur_arch::map_region(
            tos::cur_arch::get_current_translation_table(),
            identity_map(physical_segment{palloc->range_of(*recv_mem),
                                          tos::permissions::read_write}),
            tos::user_accessible::no,
            tos::memory_types::normal,
            palloc,
            mem);
        tos::ensure(map_res);

        auto buf_ptr = new (mem.direct_mapped()) buf{};

        queue_rx_buf(*buf_ptr);
    }

    transport().setup_interrupts(tos::mem_function_ref<&network_device::isr>(*this));
    transport().enable_interrupts();

    transport().write_byte(status_port_offset, 0xf);
    LOG_TRACE("Device initialized");
    return true;
}

uint32_t network_device::negotiate(uint32_t features) {
    return mac_feature | status_feature | mtu_feature | checksum_feature |
           merge_rxbuf_feature;
}

network_device::~network_device() = default;

void network_device::queue_rx_buf(buf& buffer) {
    auto& rx_queue = queue_at(0);

    auto [body_idx, body_desc] = rx_queue.alloc();

    body_desc->addr = reinterpret_cast<uintptr_t>(&buffer.header);
    body_desc->len = 1526;
    body_desc->flags = queue_flags::write;
    body_desc->next = 0;

    rx_queue.submit_available(body_idx);

    transport().write_u16(notify_port_offset, 0);
}

void network_device::isr() {
    asm volatile("push %rax");
    auto isr_status =
        transport().has_msix() ? 1 : transport().read_byte(interrupt_status_port_offset);

    if (isr_status & 1) {
        auto& rx_queue = queue_at(0);
        auto& tx_queue = queue_at(1);

        rx_queue.for_each_used([&](queue_used_elem el) {
            auto total_len = el.len;

            // The received packet consists of 2 descriptors: header + body
            auto& body_desc = rx_queue.descriptors()[el.id];

            auto buf_ptr = reinterpret_cast<buf*>(
                reinterpret_cast<char*>(body_desc.addr) - offsetof(buf, header));

            buf_ptr->header.hdr_len = total_len - sizeof buf_ptr->header;

            received_packets.push_back(*buf_ptr);
            m_rx_sem.up_isr();
        });

        tx_queue.for_each_used([&](queue_used_elem el) {
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
    tos::int_guard ig(__builtin_return_address(0));
    queue_at(0).used_base->disable_irq();
    auto& b = received_packets.front();
    received_packets.pop_front();
    queue_at(0).used_base->enable_irq();
    return b.body();
}

void network_device::return_packet(span<uint8_t> buffer) {
    auto b = reinterpret_cast<buf*>(buffer.data() - offsetof(buf, mem));
    Assert(b->header.hdr_len == buffer.size());
    queue_rx_buf(*b);
}

void network_device::transmit_gather(span<span<const uint8_t>> data) {
    if (data.empty()) {
        return;
    }

    auto& tx_queue = queue_at(1);

    tx_header header{};

    auto [header_idx, header_desc] = tx_queue.alloc();
    header_desc->addr = reinterpret_cast<uintptr_t>(&header.header);
    header_desc->len = sizeof header.header;
    header_desc->flags = queue_flags(queue_flags::next);
    header_desc->next = 0;

    queue_descriptor* cur = header_desc;

    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        auto [body_idx, body_desc] = tx_queue.alloc();

        cur->flags = queue_flags::next;
        cur->next = body_idx;

        body_desc->addr = reinterpret_cast<uintptr_t>(data.data());
        body_desc->len = data.size();
        body_desc->flags = queue_flags::none;
        body_desc->next = 0;
        cur = body_desc;
    }
    cur->flags = queue_flags::none;

    tx_queue.submit_available(header_idx);

    transport().write_u16(notify_port_offset, 1);

    header.sem.down();
}

void network_device::transmit_gather_callback_impl(
    function_ref<span<const uint8_t>()> cb) {
    auto& tx_queue = queue_at(1);

    tx_header header{};

    auto [header_idx, header_desc] = tx_queue.alloc();
    header_desc->addr = reinterpret_cast<uintptr_t>(&header.header);
    header_desc->len = sizeof header.header;
    header_desc->flags = queue_flags::none;
    header_desc->next = 0;

    queue_descriptor* cur = header_desc;

    auto data = cb();
    while (!data.empty()) {
        auto [body_idx, body_desc] = tx_queue.alloc();

        cur->flags = queue_flags::next;
        cur->next = body_idx;

        body_desc->addr = reinterpret_cast<uintptr_t>(data.data());
        body_desc->len = data.size();
        body_desc->flags = queue_flags::none;
        body_desc->next = 0;

        cur = body_desc;
        data = cb();
    }

    tx_queue.submit_available(header_idx);

    transport().write_u16(notify_port_offset, 1);

    header.sem.down();
}

Task<void> network_device::async_transmit_gather_callback_impl(
    function_ref<span<const uint8_t>()> cb) {
    auto& tx_queue = queue_at(1);

    tx_header header{};

    auto [header_idx, header_desc] = tx_queue.alloc();
    header_desc->addr = reinterpret_cast<uintptr_t>(&header.header);
    header_desc->len = sizeof header.header;
    header_desc->flags = queue_flags::none;
    header_desc->next = 0;

    queue_descriptor* cur = header_desc;

    auto data = cb();
    while (!data.empty()) {
        auto [body_idx, body_desc] = tx_queue.alloc();

        cur->flags = queue_flags::next;
        cur->next = body_idx;

        body_desc->addr = reinterpret_cast<uintptr_t>(data.data());
        body_desc->len = data.size();
        body_desc->flags = queue_flags::none;
        body_desc->next = 0;

        cur = body_desc;
        data = cb();
    }

    tx_queue.submit_available(header_idx);

    transport().write_u16(notify_port_offset, 1);

    co_await header.sem;
}

void network_device::transmit_packet(span<const uint8_t> data) {
    transmit_gather_callback([&, n = 0]() mutable {
        if (n++ == 0) {
            return data;
        }
        return span<const uint8_t>{nullptr};
    });
}

Task<void> network_device::async_transmit_packet(span<const uint8_t> data) {
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
    transport().write_u16(notify_port_offset, 1);

    co_await header.sem;
}
} // namespace tos::virtio