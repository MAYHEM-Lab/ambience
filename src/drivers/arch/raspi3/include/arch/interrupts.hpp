#pragma once

#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/aarch64/interrupts.hpp>
#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>
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

    void register_handler(int channel, irq_handler& handler) {
        m_irq_lists[channel].push_back(handler);
    }

    void synchronous(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far);
    void irq(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far);
    void fiq(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far);
    void serror(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far);

private:
    std::unordered_map<int, intrusive_list<irq_handler>> m_irq_lists;
};
} // namespace tos::raspi3