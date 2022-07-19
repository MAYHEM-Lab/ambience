#include "tos/span.hpp"
#include <tos/arm/assembly.hpp>
#include <tos/arm/cmsis.hpp>
#include <tos/arm/exception.hpp>
#include <tos/debug/debug.hpp>
#include <tos/utility.hpp>

void low_level_write(tos::span<const uint8_t>);

namespace tos::arm::exception {
namespace {
tos::function_ref<bool(const fault_variant&)> fault_handler{
    [](const fault_variant&, void*) { return false; }};
tos::function_ref<void(int, stack_frame_t&)> svc_handler{
    [](int, stack_frame_t&, void*) {}};

template<class VisitorT>
auto analyze_usage_fault(const stack_frame_t& frame, VisitorT&& visitor) {
#if defined(SCB_CFSR_UNDEFINSTR_Msk)
    auto cfsr = SCB->CFSR;
    SCB->CFSR |= SCB->CFSR;
    if (cfsr & SCB_CFSR_UNDEFINSTR_Msk) {
        return visitor(
            undefined_instruction_fault{.instr_address = frame.return_address});
    }
#endif
#if defined(SCB_CFSR_DIVBYZERO_Msk)
    if (cfsr & SCB_CFSR_DIVBYZERO_Msk) {
        return visitor(div_by_zero_fault{.instr_address = frame.return_address});
    }
#endif
    return visitor(unknown_fault{});
}

fault_variant analyze_usage_fault(const stack_frame_t& frame) {
    return analyze_usage_fault(frame,
                               [](const auto& fault) { return fault_variant(fault); });
}

template<class VisitorT>
auto analyze_memfault(const stack_frame_t& frame, VisitorT&& visitor) {
#if defined(SCB_CFSR_MMARVALID_Msk)
    auto cfsr = SCB->CFSR;
    auto mmfar = SCB->MMFAR;
    SCB->CFSR |= SCB->CFSR;

    if (cfsr & SCB_CFSR_MMARVALID_Msk) {
        return visitor(
            memory_fault{.instr_address = frame.return_address, .data_address = mmfar});
    }
#endif
    return visitor(unknown_fault{.instr_address = frame.return_address});
}

fault_variant analyze_memfault(const stack_frame_t& frame) {
    return analyze_memfault(frame,
                            [](const auto& fault) { return fault_variant(fault); });
}

template<class VisitorT>
auto analyze_bus_fault(const stack_frame_t& frame, VisitorT&& visitor) {
#if defined(SCB_CFSR_BFARVALID_Msk)
    auto cfsr = SCB->CFSR;
    auto bfar = SCB->BFAR;
    SCB->CFSR |= SCB->CFSR;

    if (cfsr & SCB_CFSR_BFARVALID_Msk) {
        return visitor(bus_fault_t{.instr_address = frame.return_address,
                                   .fault_address = bfar,
                                   .precise = (cfsr & SCB_CFSR_PRECISERR_Msk) == 0});
    }
#endif
    return visitor(unknown_fault{});
}
fault_variant analyze_bus_fault(const stack_frame_t& frame) {
    return analyze_bus_fault(frame,
                             [](const auto& fault) { return fault_variant(fault); });
}

void break_if_attached() {
    return;
#if defined(CoreDebug_DHCSR_C_DEBUGEN_Msk)
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
        breakpoint();
    }
#endif
}

extern "C" {
[[gnu::used]] void hard_fault_handler(stack_frame_t* frame) {
#if defined(SCB_HFSR_FORCED_Msk)
    auto forced = (SCB->HFSR & SCB_HFSR_FORCED_Msk) != 0;
    tos::debug::do_not_optimize(&forced);
#endif

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

[[gnu::used]] void in_svc_handler(stack_frame_t* frame) {
    auto addr = reinterpret_cast<const uint16_t*>(frame->return_address - 2);
    if (frame->r0 == 0xDEADBEEF) {
        low_level_write(*(tos::span<const uint8_t>*)frame->r1);
        return;
    }
    auto svc_num = *addr & 0xFF;
    svc_handler(svc_num, *frame);
}
}
} // namespace

fault_handler_t set_general_fault_handler(fault_handler_t handler) {
    std::swap(fault_handler, handler);
    return handler;
}

svc_handler_t set_svc_handler(svc_handler_t handler) {
    std::swap(svc_handler, handler);
    return handler;
}

//TODO: these don't work with thumb1 (i.e. armv6m (cortex M0+))

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

[[gnu::naked]] void out_svc_handler() {
    __asm volatile("tst lr, #4 \n"
                   "ite eq \n"
                   "mrseq r0, msp \n"
                   "mrsne r0, psp \n"
                   "b in_svc_handler \n");
}
} // namespace tos::arm::exception