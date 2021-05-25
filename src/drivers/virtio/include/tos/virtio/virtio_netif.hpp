#pragma once

#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/virtio/network_device.hpp>

namespace tos::virtio {
class net_if : public tos::lwip::netif_base<net_if> {
public:
    explicit net_if(tos::virtio::network_device* dev)
        : m_dev(dev) {
        add();
    }

    err_t init();

    void read_thread(tos::cancellation_token& tok);

    err_t link_output(pbuf* p);

private:
    tos::virtio::network_device* m_dev;
};
} // namespace tos::virtio