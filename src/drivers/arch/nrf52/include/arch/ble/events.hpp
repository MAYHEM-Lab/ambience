//
// Created by fatih on 6/2/19.
//

#pragma once

#include <algorithm>
#include <ble.h>
#include <nrf_sdh_ble.h>
#include <nrf_sdh_soc.h>
#include <tos/function_ref.hpp>
#include <tos/interrupt.hpp>
#include <tos/intrusive_list.hpp>

namespace tos {
namespace nrf52 {
struct soc_observer : tos::list_node<soc_observer> {
    // nrf sdk defines the observer_ts to be const by default -_-
    // so remove the const here
    std::remove_const_t<nrf_sdh_soc_evt_observer_t> obs;
};

struct ble_observer : tos::list_node<ble_observer> {
    // nrf sdk defines the observer_ts to be const by default -_-
    // so remove the const here
    std::remove_const_t<nrf_sdh_ble_evt_observer_t> obs;

    template<class FnT>
    explicit ble_observer(FnT& fn) {
        obs.handler = [](const ble_evt_t* ev, void* ctx) {
            auto ptr = static_cast<FnT*>(ctx);
            (*ptr)(*ev);
        };
        obs.p_context = &fn;
    }

    ble_observer() = default;
};

class nrf_events_t {
public:
    void attach(ble_observer& obs, const tos::no_interrupts& = tos::int_guard{}) {
        ble_observers.push_back(obs);
    }

    void attach(soc_observer& obs, const tos::no_interrupts& = tos::int_guard{}) {
        soc_observers.push_back(obs);
    }

    void remove(ble_observer& obs) {
        auto ble_it = std::find_if(ble_observers.begin(),
                                   ble_observers.end(),
                                   [&](auto& a) { return &a == &obs; });
        if (ble_it != ble_observers.end())
            ble_observers.erase(ble_it);
    }

    void remove(soc_observer& obs) {
        auto soc_it = std::find_if(soc_observers.begin(),
                                   soc_observers.end(),
                                   [&](auto& a) { return &a == &obs; });
        if (soc_it != soc_observers.end())
            soc_observers.erase(soc_it);
    }

    void on_soc_evt(uint32_t ev) {
        for (auto& obs : soc_observers) {
            obs.obs.handler(ev, obs.obs.p_context);
        }
    }

    void on_ble_evt(ble_evt_t const* ev) {
        for (auto& obs : ble_observers) {
            obs.obs.handler(ev, obs.obs.p_context);
        }
    }

private:
    tos::intrusive_list<ble_observer> ble_observers;
    tos::intrusive_list<soc_observer> soc_observers;
};

extern nrf_events_t nrf_events;
} // namespace nrf52
} // namespace tos