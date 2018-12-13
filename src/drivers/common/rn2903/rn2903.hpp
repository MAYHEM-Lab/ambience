//
// Created by fatih on 12/13/18.
//

#pragma once

#include <tos/expected.hpp>
#include <tos/devices.hpp>
#include "rn2903_common.hpp"
#include "sys.hpp"
#include "mac.hpp"

namespace tos
{
namespace rn2903
{
    template <class UartT, class AlarmT>
    class lora
    {
    public:
        constexpr lora(UartT& uart, AlarmT& alarm)
            : m_uart{&uart}, m_alarm{&alarm} {}

        constexpr expected<uint8_t, failures>
        nvm_get(nvm_addr_t addr);

        constexpr expected<void, failures>
        set_otaa_params(const dev_eui_t&, const app_eui_t&, const app_key_t&);
        constexpr expected<void, failures>
        save_mac();

        constexpr expected<join_res, failures>
        join_otaa();

    private:
        UartT* m_uart;
        AlarmT* m_alarm;
    };
} // namespace rn2903

namespace devs
{
    using rn2903_t = dev<struct _rn2903_t, 0>;
    static constexpr rn2903_t rn2903{};
} // namespace devs

template <class UartT, class AlarmT>
rn2903::lora<UartT, AlarmT> open_impl(devs::rn2903_t, UartT& uart, AlarmT& alarm)
{
    return {uart, alarm};
}
} // namespace tos

// impl

namespace tos
{
namespace rn2903
{
    template<class UartT, class AlarmT>
    constexpr expected <uint8_t, failures>
    lora<UartT, AlarmT>::nvm_get(nvm_addr_t addr) {
        return rn2903::nvm_get(addr, *m_uart, *m_alarm);
    }

    template<class UartT, class AlarmT>
    constexpr expected<void, failures>
    lora<UartT, AlarmT>::save_mac() {
        using namespace std::chrono_literals;
        tos::println(*m_uart, "mac save");

        char buf[5] {};
        using namespace std::chrono_literals;
        auto r = m_uart->read(buf, *m_alarm, 5s);

        if (r.size() == 0)
        {
            return unexpected(failures::timeout);
        }

        return {};
    }

    template<class UartT, class AlarmT>
    constexpr expected<join_res, failures> lora<UartT, AlarmT>::join_otaa() {
        return rn2903::join_otaa(*m_uart, *m_alarm);
    }
} // namespace rn2903
} // namespace tos