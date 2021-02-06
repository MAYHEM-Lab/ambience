#pragma once

#include <tos/virtio/common.hpp>
#include <tos/virtio/queue.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos::virtio {
class device {
public:
    explicit device(x86_64::pci::device&& pci_dev);

    virtual bool initialize(tos::physical_page_allocator* palloc) = 0;
    virtual ~device() = default;

protected:
    uint32_t bar_base() const;

    bool base_initialize(tos::physical_page_allocator* palloc);

    virtual uint32_t negotiate(uint32_t features) = 0;

    queue& queue_at(int idx) {
        return m_queues[idx];
    }

    tos::x86_64::pci::device& pci_dev() {
        return m_pci_dev;
    }

    static constexpr uint32_t ring_event_idx = 1 << 29;

private:
    struct capability_data {
        capability_type type;
        uint8_t bar;
        uint32_t offset;
        uint32_t length;
    };

    void handle_capability(x86_64::pci::capability& cap);

    capability_data* m_common;
    capability_data* m_pci;
    std::vector<capability_data> m_capabilities;
    x86_64::pci::device m_pci_dev;
    std::vector<queue> m_queues;
    uint16_t m_bar_base;
};
} // namespace tos::virtio