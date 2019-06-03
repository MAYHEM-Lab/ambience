//
// Created by fatih on 6/2/19.
//

#include <arch/ble.hpp>

using namespace tos::nrf52;

static nrf_sdh_soc_evt_observer_t tos_soc_observer
    __attribute__((section(".sdh_soc_observers1"))) __attribute__((used))
    { [](uint32_t ev, void* ctx) {
        auto events = (nrf_events_t*) ctx;
        events->on_soc_evt(ev);
    }, &nrf_events };

static nrf_sdh_ble_evt_observer_t tos_ble_observer
    __attribute__((section(".sdh_ble_observers1"))) __attribute__((used))
    { [](ble_evt_t const* ev, void* ctx) {
        auto events = (nrf_events_t*) ctx;
        events->on_ble_evt(ev);
    }, &nrf_events };

namespace tos::nrf52{
nrf_events_t nrf_events;
}