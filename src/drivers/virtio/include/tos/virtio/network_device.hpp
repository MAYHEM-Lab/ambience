#pragma once

#include <cstdint>
#include <tos/virtio/device.hpp>
#include <common/inet/tcp_ip.hpp>

namespace tos::virtio {
class network_device : public device {
public:
    using device::device;

    bool initialize(tos::physical_page_allocator* palloc) override;
    mac_addr_t address() const;
    size_t mtu() const;
protected:

    
    uint32_t negotiate(uint32_t) override;

private:

    static constexpr auto checksum_feature = 1 << 0;
    static constexpr auto mtu_feature = 1 << 3;
    static constexpr auto mac_feature = 1 << 5;
    static constexpr auto status_feature = 1 << 16;
    static constexpr auto control_queue_feature = 1 << 17;
    static constexpr auto multiqueue_feature = 1 << 22;
};
} // namespace tos::virtio