#include <tos/detail/poll.hpp>
#include <tos/virtio/virtio_netif.hpp>

namespace tos::virtio {
err_t net_if::init() {
    LOG("In init");
    pre_init();
    m_if.hostname = "ambience";
    m_if.flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
    m_if.name[1] = m_if.name[0] = 'm';
    m_if.num = 0;
    m_if.mtu = 1514;
    LOG("MTU", m_if.mtu);
    m_if.hwaddr_len = 6;
    auto mac = m_dev->address();
    memcpy(m_if.hwaddr, mac.addr.data(), 6);
    auto& t = launch(tos::alloc_stack,
                     [this] { read_thread(tos::cancellation_token::system()); });
    set_name(t, "LWIP Read Thread");
    return ERR_OK;
}

void net_if::read_thread(cancellation_token& tok) {
    bool printed = false;
    //        tos::debug::log("In read thread");
    while (!tok.is_cancelled()) {
        auto packet = m_dev->take_packet();

        //            tos::debug::log("Got packet", this, packet.size());
        // auto p = pbuf_alloc(pbuf_layer::PBUF_RAW, packet.size(), pbuf_type::PBUF_POOL);
        struct zerocopy_rx_data : pbuf_custom {
            span<uint8_t> packet;
            tos::virtio::network_device* netdev;

            ~zerocopy_rx_data() {
                netdev->return_packet(packet);
            }
        };
        auto custom_pbuf = new zerocopy_rx_data;
        custom_pbuf->packet = packet;
        custom_pbuf->netdev = m_dev;
        custom_pbuf->custom_free_function = [](pbuf* data) {
            delete reinterpret_cast<zerocopy_rx_data*>(data);
        };
        auto p = pbuf_alloced_custom(pbuf_layer::PBUF_RAW,
                                     packet.size_bytes(),
                                     pbuf_type::PBUF_POOL,
                                     custom_pbuf,
                                     packet.data(),
                                     packet.size_bytes());
        if (!p) {
            tos::debug::error("Could not allocate pbuf!", packet.size());
            m_dev->return_packet(packet);
            continue;
        }

        // tos::lwip::copy_to_pbuf(packet.begin(), packet.end(), *p);
        // m_dev->return_packet(packet);

        {
            tos::lock_guard lg{tos::lwip::lwip_lock};
            m_if.input(p, &m_if);
        }

        if (!printed && dhcp_supplied_address(&m_if)) {
            printed = true;
            auto addr = tos::lwip::convert_to_tos(m_if.ip_addr);
            tos::debug::log("Got addr!",
                            (int)addr.addr[0],
                            (int)addr.addr[1],
                            (int)addr.addr[2],
                            (int)addr.addr[3]);
        }
    }
}

err_t net_if::link_output(pbuf* p) {
    // Keep buffer alive during transmit, we'll return early
    pbuf_ref(p);

    tos::coro::make_detached(m_dev->async_transmit_gather_callback([p]() mutable {
        if (!p) {
            return tos::span<const uint8_t>(nullptr);
        }
        auto res =
            tos::span<const uint8_t>{static_cast<const uint8_t*>(p->payload), p->len};
        p = p->next;
        return res;
    }),
                             [p] { pbuf_free(p); });
    return ERR_OK;
}
} // namespace tos::virtio