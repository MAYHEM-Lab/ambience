//
// Created by fatih on 12/10/18.
//

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <common/driver_base.hpp>
#include <common/timer.hpp>
#include <tos/arch.hpp>
#include <tos/function_ref.hpp>

namespace tos {
namespace x86 {
class timer
    : public self_pointing<timer>
    , public non_copy_movable {
public:
    timer()
        : m_tmr{get_io()}
        , m_cb{[](void*) {}}
        , m_interval{0} {
    }

    void set_frequency(uint16_t hertz) {
        m_interval = std::chrono::milliseconds(1000 / hertz);
    }

    void set_callback(tos::function_ref<void()> fr) {
        m_cb = fr;
    }

    void enable() {
        m_disabled = false;
        m_tmr.expires_from_now(m_interval);
        m_tmr.async_wait([this](auto& ec) { handle(ec); });
    }

    void disable() {
        m_disabled = true;
        m_tmr.cancel();
    }

private:
    void handle(const boost::system::error_code& ec) {
        if (ec) {
            return;
        }

        m_cb();
        if (!m_disabled) {
            m_tmr.expires_from_now(m_interval);
            m_tmr.async_wait([this](auto& ec) { handle(ec); });
        }
    }

    bool m_disabled = true;
    boost::asio::high_resolution_timer m_tmr;
    tos::function_ref<void()> m_cb;
    std::chrono::milliseconds m_interval;
};
} // namespace x86

x86::timer open_impl(devs::timer_t<0>) {
    return {};
}
} // namespace tos