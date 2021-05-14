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

    err_t init() {
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

    void read_thread(tos::cancellation_token& tok) {
        bool printed = false;
        LOG("In read thread");
        while (!tok.is_cancelled()) {
            auto packet = m_dev->take_packet();

            auto p =
                pbuf_alloc(pbuf_layer::PBUF_RAW, packet.size(), pbuf_type::PBUF_POOL);
            if (!p) {
                LOG_ERROR("Could not allocate pbuf!");
                m_dev->return_packet(packet);
                continue;
            }

            tos::lwip::copy_to_pbuf(packet.begin(), packet.end(), *p);
            m_dev->return_packet(packet);

            {
                tos::lock_guard lg{tos::lwip::lwip_lock};
                m_if.input(p, &m_if);
            }

            if (!printed && dhcp_supplied_address(&m_if)) {
                printed = true;
                auto addr = tos::lwip::convert_to_tos(m_if.ip_addr);
                LOG("Got addr!",
                    (int)addr.addr[0],
                    (int)addr.addr[1],
                    (int)addr.addr[2],
                    (int)addr.addr[3]);
            }
        }
    }

    err_t link_output(pbuf* p) {
        LOG_TRACE("link_output", p->len, p->tot_len, "bytes");
        m_dev->transmit_gather_callback([p]() mutable {
            if (!p) {
                return tos::span<const uint8_t>(nullptr);
            }
            auto res =
                tos::span<const uint8_t>{static_cast<const uint8_t*>(p->payload), p->len};
            p = p->next;
            return res;
        });
        return ERR_OK;
    }

private:
    tos::virtio::network_device* m_dev;
};
} // namespace tos::virtio