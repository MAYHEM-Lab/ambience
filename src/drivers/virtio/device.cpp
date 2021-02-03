#include <tos/debug/log.hpp>
#include <tos/virtio/device.hpp>

namespace tos::virtio {
device::device(x86_64::pci::device&& pci_dev)
    : m_pci_dev{std::move(pci_dev)} {
    for (auto cap = m_pci_dev.capabilities_root(); cap; cap = cap->next()) {
        LOG((void*)(uintptr_t)cap->vendor(),
            (void*)(uintptr_t)cap->next()->m_offset,
            (void*)(uintptr_t)cap->len());

        if (cap->vendor() == 0x9) {
            handle_capability(*cap);
        }
    }
}

bool device::base_initialize(tos::physical_page_allocator* palloc) {
    auto bar_base = this->bar_base();
    auto status_port = x86_64::port(bar_base + status_port_offset);

    // The spec says a device MUST initialize this register to 0.
    // However, not setting this register prevents proper initialization for some
    // reason.
    status_port.outb(0);
    status_port.outb(0x1);
    status_port.outb(0x3);

    auto dev_features_port = x86_64::port(bar_base + dev_features_port_offset);
    auto features = dev_features_port.inl();

    auto driver_features_port = x86_64::port(bar_base + drv_features_port_offset);
    auto accepted_features = negotiate(features);
    LOG("Accepted features:", (void*)accepted_features);
    driver_features_port.outl(accepted_features);

    status_port.outb(0xB);

    auto resp = status_port.inb();
    if ((resp & 0x8) == 0) {
        LOG_ERROR("Feature negotiation failed");
        return false;
    }
    LOG_TRACE("Features negotiated");

    auto queue_sel_port = x86_64::port(bar_base + queue_sel_port_offset);
    auto queue_sz_port = x86_64::port(bar_base + queue_size_port_offset);
    auto queue_base_port = x86_64::port(bar_base + queue_addr_port_offset);

    for (int i = 0; i < 16; ++i) {
        queue_sel_port.outw(i);
        auto sz = queue_sz_port.inw();
        if (sz == 0) {
            continue;
        }
        LOG_TRACE("Queue", i, "size", sz);
        m_queues.emplace_back(sz, *palloc);
        auto queue_base =
            reinterpret_cast<uintptr_t>(m_queues.back().descriptors_base) / 4096;
        queue_base_port.outl(queue_base);
        LOG(int(queue_base));
    }

    return true;
}

void device::handle_capability(x86_64::pci::capability& cap) {
    // Virtio vendor capability
    LOG_TRACE("Type:",
              (void*)(uintptr_t)cap.read_byte(3),
              "BAR:",
              (void*)(uintptr_t)cap.read_byte(4),
              "Offset:",
              (void*)(uintptr_t)cap.read_long(8),
              "Length:",
              (void*)(uintptr_t)cap.read_long(12));

    capability_data data;
    data.type = static_cast<capability_type>(cap.read_byte(3));
    data.bar = cap.read_byte(4);
    data.offset = cap.read_long(8);
    data.length = cap.read_long(12);
    m_capabilities.emplace_back(data);

    if (data.type == capability_type::pci) {
        m_pci = &m_capabilities.back();
    }
}
} // namespace tos::virtio