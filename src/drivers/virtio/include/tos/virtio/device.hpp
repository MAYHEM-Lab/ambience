#pragma once

#include <tos/virtio/common.hpp>
#include <tos/virtio/queue.hpp>
#include <tos/virtio/transport.hpp>

namespace tos::virtio {
class device {
public:
    explicit device(std::unique_ptr<transport> transp);

    virtual bool initialize(tos::physical_page_allocator* palloc) = 0;
    virtual ~device() = default;

protected:
    bool base_initialize(tos::physical_page_allocator* palloc);

    virtual uint32_t negotiate(uint32_t features) = 0;

    queue& queue_at(int idx) {
        return m_queues[idx];
    }

    static constexpr uint32_t ring_event_idx = 1 << 29;

    virtio::transport& transport() const {
        return *m_transport;
    }

    bool have_msix() const {
        return m_have_msix;
    }
private:
    std::unique_ptr<virtio::transport> m_transport;
    std::vector<queue> m_queues;
    bool m_have_msix;
};
} // namespace tos::virtio