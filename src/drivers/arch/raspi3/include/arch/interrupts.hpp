#pragma once

#include <cstdint>
#include <tos/intrusive_list.hpp>
#include <tos/function_ref.hpp>
#include <unordered_map>
#include <common/driver_base.hpp>

namespace tos::raspi3 {
struct irq_handler : list_node<irq_handler> {
    irq_handler(function_ref<bool()> fn) : function(fn) {}

    function_ref<bool()> function;
};

class interrupt_controller : public tracked_driver<interrupt_controller, 1> {
public:
    interrupt_controller() : tracked_driver(0) {}

    void register_handler(int channel, irq_handler& handler) {
        m_irq_lists[channel].push_back(handler);
    }

    void irq(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far);

private:
    std::unordered_map<int, intrusive_list<irq_handler>> m_irq_lists;
};
} // namespace tos::raspi3