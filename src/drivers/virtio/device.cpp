#include <tos/debug/log.hpp>
#include <tos/virtio/device.hpp>

namespace tos::virtio {
device::device(std::unique_ptr<virtio::transport> transp)
    : m_transport(std::move(transp)) {
}

bool device::base_initialize(tos::physical_page_allocator* palloc) {
    // The spec says a device MUST initialize this register to 0.
    // However, not setting this register prevents proper initialization for some
    // reason.
    transport().write_byte(status_port_offset, 0);
    transport().write_byte(status_port_offset, 0x1);
    transport().write_byte(status_port_offset, 0x3);
    LOG_TRACE("Device status:", transport().read_byte(status_port_offset));

    auto features = transport().read_u32(dev_features_port_offset);
    LOG_TRACE("Offered features:", (void*)(uintptr_t)features);

    auto accepted_features = negotiate(features);
    LOG_TRACE("Accepted features:", (void*)(uintptr_t)accepted_features);
    transport().write_u32(drv_features_port_offset, accepted_features);

    transport().write_byte(status_port_offset, 0xB);

    auto resp = transport().read_byte(status_port_offset);
    if ((resp & 0x8) == 0) {
        LOG_ERROR("Feature negotiation failed");
        return false;
    }
    LOG_TRACE("Features negotiated", resp);

    for (int i = 0; i < 2; ++i) {
        transport().write_u16(queue_sel_port_offset, i);
        auto sz = transport().read_u16(queue_size_port_offset);
        if (sz == 0) {
            continue;
        }
        LOG_TRACE("Queue", i, "size", sz);
        m_queues.emplace_back(sz, *palloc);
        auto queue_base =
            reinterpret_cast<uintptr_t>(m_queues.back().descriptors_base) / 4096;
        transport().write_u32(queue_addr_port_offset, queue_base);
        LOG(int(queue_base));
    }

    return true;
}
} // namespace tos::virtio