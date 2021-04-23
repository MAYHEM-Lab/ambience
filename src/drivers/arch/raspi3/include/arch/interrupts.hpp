#pragma once

#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/aarch64/interrupts.hpp>
#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/soc/bcm283x.hpp>
#include <unordered_map>

namespace tos::raspi3 {
struct irq_handler : list_node<irq_handler> {
    explicit irq_handler(function_ref<bool()> fn)
        : function(fn) {
    }

    function_ref<bool()> function;
};

class interrupt_controller : public tracked_driver<interrupt_controller, 1> {
public:
    interrupt_controller()
        : tracked_driver(0) {
    }

    void register_handler(bcm283x::irq_channels channel, irq_handler& handler) {
        m_irq_lists[static_cast<int>(channel)].push_back(handler);
    }

    void irq();

    void set_post_irq(function_ref<void()> handler);
    void reset_post_irq();

private:
    void do_irq(int channel);
    bool try_irq(int channel);

    std::unordered_map<int, intrusive_list<irq_handler>> m_irq_lists;
    function_ref<void()> m_post_irq{[](void*) {}};
};
} // namespace tos::raspi3