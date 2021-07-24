#pragma once

#include "tos/semaphore.hpp"
#include <common/inet/tcp_ip.hpp>
#include <cstdint>
#include <tos/task.hpp>
#include <tos/virtio/device.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::virtio {
namespace {
struct buf;
}
class network_device : public device {
public:
    using device::device;

    bool initialize(tos::physical_page_allocator* palloc) override;
    mac_addr_t address() const;
    size_t mtu() const;

    ~network_device();

    // The returned span must be returned to the network device as is.
    span<uint8_t> take_packet();
    void return_packet(span<uint8_t>);

    template<class Fn>
    void transmit_gather_callback(Fn&& fn) {
        this->transmit_gather_callback_impl(function_ref<span<const uint8_t>()>(fn));
    }

    template<class Fn>
    Task<void> async_transmit_gather_callback(Fn&& fn) {
        return this->async_transmit_gather_callback_impl(
            function_ref<span<const uint8_t>()>(fn));
    }

    void transmit_gather(span<span<const uint8_t>> data);
    void transmit_packet(span<const uint8_t> data);
    Task<void> async_transmit_packet(span<const uint8_t> data);

protected:
    uint32_t negotiate(uint32_t) override;

private:
    void transmit_gather_callback_impl(function_ref<span<const uint8_t>()> cb);
    Task<void>
    async_transmit_gather_callback_impl(function_ref<span<const uint8_t>()> cb);
    void isr();

    void queue_rx_buf(buf&);

    tos::semaphore m_rx_sem{0};
    static constexpr auto checksum_feature = 1 << 0;
    static constexpr auto mtu_feature = 1 << 3;
    static constexpr auto mac_feature = 1 << 5;
    static constexpr auto merge_rxbuf_feature = 1 << 15;
    static constexpr auto status_feature = 1 << 16;
    static constexpr auto control_queue_feature = 1 << 17;
    static constexpr auto multiqueue_feature = 1 << 22;
};
} // namespace tos::virtio