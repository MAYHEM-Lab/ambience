#pragma once

#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <numeric>
#include <tos/cancellation_token.hpp>
#include <tos/ft.hpp>
#include <tos/lwip/utility.hpp>

namespace tos::lwip {
template<class DeviceT>
class basic_interface {
public:
    basic_interface(DeviceT&& dev,
                    const ipv4_addr_t& addr,
                    const ipv4_addr_t& mask,
                    const ipv4_addr_t& gw)
        : m_tap(std::move(dev)) {
        auto lwip_addr = convert_address(addr);
        auto lwip_mask = convert_address(mask);
        auto lwip_gw = convert_address(gw);
        netif_add(&m_if,
                  &lwip_addr,
                  &lwip_mask,
                  &lwip_gw,
                  this,
                  &basic_interface::init,
                  netif_input);
    }

    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
    }

    void down() {
        netif_set_down(&m_if);
        netif_set_link_down(&m_if);
    }

    ~basic_interface() {
        down();
        netif_remove(&m_if);
    }

private:
    netif m_if;
    DeviceT m_tap;

    err_t init() {
        m_if.hostname = "tos";
        m_if.flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        m_if.linkoutput = &basic_interface::link_output;
        m_if.output = etharp_output;
        m_if.name[1] = m_if.name[0] = 'm';
        m_if.num = 0;
        m_if.mtu = 1500;
        m_if.hwaddr_len = 6;
        m_if.link_callback = &basic_interface::link_callback;
        m_if.status_callback = &basic_interface::status_callback;
        std::iota(std::begin(m_if.hwaddr), std::end(m_if.hwaddr), 1);
        launch(alloc_stack, [this] { read_thread(); });
        return ERR_OK;
    }

    void read_thread() {
        auto& tok = tos::cancellation_token::system();
        while (!tok.is_cancelled()) {
            std::vector<uint8_t> buf(1500);
            auto recvd = m_tap->read(buf);
            if (recvd.empty()) {
                continue;
            }
            LOG_TRACE("Received", recvd.size(), "bytes");
            auto p =
                pbuf_alloc(pbuf_layer::PBUF_LINK, recvd.size(), pbuf_type::PBUF_POOL);
            std::copy(recvd.begin(), recvd.end(), static_cast<uint8_t*>(p->payload));
            m_if.input(p, &m_if);
        }
    }

    err_t link_output(pbuf* p) {
        LOG_TRACE("link_output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        auto written = m_tap->write({static_cast<const uint8_t*>(p->payload), p->len});
        LOG_TRACE("Written", written, "bytes");
        return ERR_OK;
    }

    err_t output(pbuf* p, [[maybe_unused]] const ip4_addr_t* ipaddr) {
        LOG_TRACE("output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        return ERR_OK;
    }

    void link_callback() {
    }

    void status_callback() {
    }

private:
    static err_t init(struct netif* netif) {
        return static_cast<basic_interface*>(netif->state)->init();
    }

    static err_t link_output(struct netif* netif, struct pbuf* p) {
        return static_cast<basic_interface*>(netif->state)->link_output(p);
    }

    static err_t output(struct netif* netif, struct pbuf* p, const ip4_addr_t* ipaddr) {
        return static_cast<basic_interface*>(netif->state)->output(p, ipaddr);
    }

    static void link_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->link_callback();
    }

    static void status_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->status_callback();
    }

    friend void set_default(basic_interface& interface) {
        netif_set_default(&interface.m_if);
    }
};
} // namespace tos::lwip
