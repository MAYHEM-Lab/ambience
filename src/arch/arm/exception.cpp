#include <tos/arm/assembly.hpp>
#include <tos/arm/cmsis.hpp>
#include <tos/arm/exception.hpp>
#include <tos/debug/debug.hpp>
#include <tos/utility.hpp>

namespace tos::arm::exception {
namespace {
tos::function_ref<bool(const fault_variant&)> fault_handler{
    [](const fault_variant&, void*) { return false; }};

template<class VisitorT>
auto analyze_usage_fault(const stack_frame_t& frame, VisitorT&& visitor) {
    auto cfsr = SCB->CFSR;
    SCB->CFSR |= SCB->CFSR;
    if (cfsr & SCB_CFSR_UNDEFINSTR_Msk) {
        return visitor(
            undefined_instruction_fault{.instr_address = frame.return_address});
    }
    if (cfsr & SCB_CFSR_DIVBYZERO_Msk) {
        return visitor(div_by_zero_fault{.instr_address = frame.return_address});
    }
    return visitor(unknown_fault{});
}

fault_variant analyze_usage_fault(const stack_frame_t& frame) {
    return analyze_usage_fault(frame,
                               [](const auto& fault) { return fault_variant(fault); });
}

template<class VisitorT>
auto analyze_memfault(const stack_frame_t& frame, VisitorT&& visitor) {
    auto cfsr = SCB->CFSR;
    auto mmfar = SCB->MMFAR;
    SCB->CFSR |= SCB->CFSR;

    if (cfsr & SCB_CFSR_MMARVALID_Msk) {
        return visitor(
            memory_fault{.instr_address = frame.return_address, .data_address = mmfar});
    }

    return visitor(unknown_fault{.instr_address = frame.return_address});
}

fault_variant analyze_memfault(const stack_frame_t& frame) {
    return analyze_memfault(frame,
                            [](const auto& fault) { return fault_variant(fault); });
}

template<class VisitorT>
auto analyze_bus_fault(const stack_frame_t& frame, VisitorT&& visitor) {
    auto cfsr = SCB->CFSR;
    auto bfar = SCB->BFAR;
    SCB->CFSR |= SCB->CFSR;

    if (cfsr & SCB_CFSR_BFARVALID_Msk) {
        return visitor(bus_fault_t{.instr_address = frame.return_address,
                                   .fault_address = bfar,
                                   .precise = (cfsr & SCB_CFSR_PRECISERR_Msk) == 0});
    }

    return visitor(unknown_fault{});
}
fault_variant analyze_bus_fault(const stack_frame_t& frame) {
    return analyze_bus_fault(frame,
                             [](const auto& fault) { return fault_variant(fault); });
}

void break_if_attached() {
//    return;
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
        breakpoint();
    }
}

extern "C" {
[[gnu::used]] void hard_fault_handler(stack_frame_t* frame) {
    auto forced = (SCB->HFSR & SCB_HFSR_FORCED_Msk) != 0;
    tos::debug::do_not_optimize(&forced);

    auto fault = analyze_usage_fault(*frame);
    break_if_attached();

    auto fault_from_isr = (frame->xpsr & IPSR_ISR_Msk) != 0;

    if (fault_from_isr) {
        // reset
    }

    if (!fault_handler(fault)) {
        breakpoint();
    }
}

[[gnu::used]] void bus_fault_handler(stack_frame_t* frame) {
    auto fault = analyze_bus_fault(*frame);
    break_if_attached();
    if (!fault_handler(fault)) {
        breakpoint();
    }
}

[[gnu::used]] void mem_fault_handler(stack_frame_t* frame) {
    auto fault = analyze_memfault(*frame);
    break_if_attached();
    if (!fault_handler(fault)) {
        breakpoint();
    }
}

[[gnu::used]] void usage_fault_handler(stack_frame_t* frame) {
    auto fault = analyze_usage_fault(*frame);
    tos::debug::do_not_optimize(&fault);
    break_if_attached();
    if (!fault_handler(fault)) {
        breakpoint();
    }
}
}
} // namespace

void set_general_fault_handler(tos::function_ref<bool(const fault_variant&)> handler) {
    fault_handler = handler;
}

[[gnu::naked]] void hard_fault() {
    __asm volatile("tst lr, #4 \n"
                   "ite eq \n"
                   "mrseq r0, msp \n"
                   "mrsne r0, psp \n"
                   "b hard_fault_handler \n");
}

[[gnu::naked]] void mem_fault() {
    __asm volatile("tst lr, #4 \n"
                   "ite eq \n"
                   "mrseq r0, msp \n"
                   "mrsne r0, psp \n"
                   "b mem_fault_handler \n");
}

[[gnu::naked]] void bus_fault() {
    __asm volatile("tst lr, #4 \n"
                   "ite eq \n"
                   "mrseq r0, msp \n"
                   "mrsne r0, psp \n"
                   "b bus_fault_handler \n");
}

[[gnu::naked]] void usage_fault() {
    __asm volatile("tst lr, #4 \n"
                   "ite eq \n"
                   "mrseq r0, msp \n"
                   "mrsne r0, psp \n"
                   "b usage_fault_handler \n");
}
} // namespace tos::arm::exception