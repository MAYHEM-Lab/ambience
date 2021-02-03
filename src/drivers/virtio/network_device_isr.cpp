#include <tos/virtio/network_device.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/debug/log.hpp>

namespace tos::virtio{
struct virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
};

struct network_device::buffer {
    virtio_net_hdr* header;
    void* data;
};

void network_device::isr(tos::x86_64::exception_frame* f, int num) {
    asm volatile("push %rax");
    auto bar_base = this->bar_base();
    auto isr_status_port = x86_64::port(bar_base + interrupt_status_port_offset);
    auto isr_status = isr_status_port.inb();
    LOG("ISR:", int(isr_status));
    if (isr_status & 1) {
        LOG("We got something!", m_buffers.size());

        auto buf = std::move(m_buffers.front());
        auto& used = queue_at(0).used_base;
        LOG(used->index, used->ring[used->index - 1].id, used->ring[used->index - 1].len);
        auto& desc = queue_at(0).descriptors()[used->ring[used->index - 1].id];
        auto header = (virtio_net_hdr*)buf.data;
        LOG(buf.data);
        LOG((void*)header->flags, header->num_buffers, header->hdr_len);
        LOG(tos::span<uint8_t>((uint8_t*)desc.addr, used->ring[used->index - 1].len));
        LOG(std::string_view((char*)desc.addr, used->ring[used->index - 1].len));

        *buf.header = {};
        m_buffers.erase(m_buffers.begin());
        queue_rx_buf(std::move(buf));
    }
    asm volatile("pop %rax");   
}
} // namespace tos::virtio