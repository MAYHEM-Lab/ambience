//
// Created by fatih on 10/14/19.
//

#pragma once

#include "gpio.hpp"

namespace tos {
namespace avr {
class exti
    : public self_pointing<exti>
    , public tracked_driver<exti, 1> {
public:
    exti()
        : tracked_driver(0){};
    void attach(const pin_t& pin, pin_change::rising_t, function_ref<void()> handler);
    void attach(const pin_t& pin, pin_change::falling_t, function_ref<void()> handler);

    void isr(int num);

private:
    void
    attach(const pin_t& pin, pin_change_values val, tos::function_ref<void()> handler);

    tos::function_ref<void()> m_exint_handlers[2]{function_ref<void()>{[](void*) {}},
                                                  function_ref<void()>{[](void*) {}}};
};
} // namespace avr
} // namespace tos

// Impl

namespace tos {
namespace avr {
#if defined(EICRA)
inline void
exti::attach(const pin_t& pin, pin_change_values val, tos::function_ref<void()> handler) {
    if (pin.port != ports::D())
        return;
    if (pin.pin != 2 && pin.pin != 3)
        return;
    auto mask = uint8_t(val);
    mask <<= ((pin.pin - 2) * 2);
    m_exint_handlers[pin.pin - 2] = handler;
    EICRA |= mask;
    EIMSK |= (1 << (pin.pin - 2));
}

inline void
exti::attach(const pin_t& pin, pin_change::rising_t, tos::function_ref<void()> handler) {
    attach(pin, pin_change_values::rising, handler);
}

inline void
exti::attach(const pin_t& pin, pin_change::falling_t, tos::function_ref<void()> handler) {
    attach(pin, pin_change_values::falling, handler);
}
#endif

void exti::isr(int num) {
    m_exint_handlers[num]();
}
} // namespace avr
} // namespace tos