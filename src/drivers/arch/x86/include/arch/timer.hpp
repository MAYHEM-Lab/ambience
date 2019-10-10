//
// Created by fatih on 12/10/18.
//

#pragma once

#include <tos/arch.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <tos/function_ref.hpp>
#include <common/driver_base.hpp>
#include <common/timer.hpp>

namespace tos
{
namespace x86
{
    class timer : public self_pointing<timer>
    {
    public:
        timer() : m_tmr{get_io()}, m_interval{0}, m_cb{[](void*){}} {}

        void set_frequency(uint16_t hertz)
        {
            m_interval = std::chrono::milliseconds(1000 / hertz);
        }

        void set_callback(tos::function_ref<void()> fr) {
            m_cb = fr;
        }

        void enable()
        {
            m_tmr.expires_from_now(m_interval);
            m_tmr.async_wait([this](auto& ec) { handle(ec); });
        }

        void disable()
        {
            m_tmr.cancel();
        }

    private:

        void handle(const boost::system::error_code& ec){
            if (ec) return;
            m_cb();
            m_tmr.expires_from_now(m_interval);
            m_tmr.async_wait([this](auto& ec) { handle(ec); });
        }

        boost::asio::high_resolution_timer m_tmr;
        tos::function_ref<void()> m_cb;
        std::chrono::milliseconds m_interval;
    };
} // namespace x86

x86::timer open_impl(devs::timer_t<0>) {
    return {};
}

} // namespace tos