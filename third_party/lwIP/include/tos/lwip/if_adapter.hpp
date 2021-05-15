#pragma once

#include "lwip.hpp"
#include <lwip/dhcp.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <numeric>
#include <tos/cancellation_token.hpp>
#include <tos/ft.hpp>
#include <tos/lwip/utility.hpp>

namespace tos::lwip {
template<class ImplT>
class netif_base {
public:
    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
        if (auto res = dhcp_start(&m_if); res != ERR_OK) {
            LOG_ERROR("No DHCP!", res);
        } else {
            LOG("DHCP Started");
        }
    }

    void down() {
        netif_set_down(&m_if);
        netif_set_link_down(&m_if);
    }

    ~netif_base() {
        down();
        netif_remove(&m_if);
    }

    err_t output(pbuf* p, [[maybe_unused]] const ip4_addr_t* ipaddr) {
        return ERR_OK;
    }

    void link_callback() {
    }

    void status_callback() {
    }

protected:
    void add() {
        netif_add(&m_if,
                  nullptr,
                  nullptr,
                  nullptr,
                  static_cast<ImplT*>(this),
                  &netif_base::init,
                  netif_input);
    }

    void pre_init() {
        set_callbacks();
        m_if.output = etharp_output;
    }

    netif m_if;

private:
    void set_callbacks() {
        m_if.linkoutput = &netif_base::link_output;
        m_if.link_callback = &netif_base::link_callback;
        m_if.status_callback = &netif_base::status_callback;
    }

    static err_t init(struct netif* netif) {
        return static_cast<ImplT*>(netif->state)->init();
    }

    static err_t link_output(struct netif* netif, struct pbuf* p) {
        return static_cast<ImplT*>(netif->state)->link_output(p);
    }

    static err_t output(struct netif* netif, struct pbuf* p, const ip4_addr_t* ipaddr) {
        return static_cast<ImplT*>(netif->state)->output(p, ipaddr);
    }

    static void link_callback(struct netif* netif) {
        static_cast<ImplT*>(netif->state)->link_callback();
    }

    static void status_callback(struct netif* netif) {
        static_cast<ImplT*>(netif->state)->status_callback();
    }

    friend void set_default(ImplT& interface) {
        netif_set_default(&interface.m_if);
    }
};

template<class DeviceT>
class basic_interface : public netif_base<basic_interface<DeviceT>> {
public:
    explicit basic_interface(DeviceT&& dev)
        : m_tap(std::move(dev)) {
        this->add();
    }

    err_t init() {
        this->pre_init();
        this->m_if.hostname = "tos";
        this->m_if.flags |=
            NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        this->m_if.name[1] = this->m_if.name[0] = 'm';
        this->m_if.num = 0;
        this->m_if.mtu = 1514;
        this->m_if.hwaddr_len = 6;
        auto mac = m_tap->address();
        memcpy(this->m_if.hwaddr, mac.addr.data(), 6);
        auto& t = launch(tos::alloc_stack,
                         [this] { read_thread(tos::cancellation_token::system()); });
        set_name(t, "LWIP Read Thread");
        return ERR_OK;
    }

    void read_thread(tos::cancellation_token& tok) {
        bool printed = false;
        LOG("In read thread");
        while (!tok.is_cancelled()) {
            std::vector<uint8_t> buf(1500);
            auto recvd = m_tap->read(buf);
            if (recvd.empty()) {
                continue;
            }
            LOG_TRACE("Received", recvd.size(), "bytes");

            auto p =
                pbuf_alloc(pbuf_layer::PBUF_RAW, recvd.size(), pbuf_type::PBUF_POOL);
            if (!p) {
                LOG_ERROR("Could not allocate pbuf!");
                continue;
            }

            tos::lwip::copy_to_pbuf(recvd.begin(), recvd.end(), *p);

            {
                tos::lock_guard lg{tos::lwip::lwip_lock};
                this->m_if.input(p, &this->m_if);
            }

            if (!printed && dhcp_supplied_address(&this->m_if)) {
                printed = true;
                auto addr = tos::lwip::convert_to_tos(this->m_if.ip_addr);
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
        auto written = m_tap->write({static_cast<const uint8_t*>(p->payload), p->len});
        LOG_TRACE("Written", written, "bytes");
        return ERR_OK;
    }

private:
    DeviceT m_tap;
};
} // namespace tos::lwip
