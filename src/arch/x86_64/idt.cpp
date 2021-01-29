#include <tos/x86_64/idt.hpp>

namespace tos::x86_64 {
interrupt_descriptor_table default_idt() {
    interrupt_descriptor_table idt;
    idt.div_by_zero = idt_entry::create(_div_by_zero_handler);
    idt.debug = idt_entry::create(_debug_handler);
    idt.nmi = idt_entry::create(_nmi_handler);
    idt.breakpoint = idt_entry::create(_breakpoint_handler);
    idt.overflow = idt_entry::create(_overflow_handler);
    idt.out_of_bounds = idt_entry::create(_out_of_bounds_handler);
    idt.invalid_opcode = idt_entry::create(_invalid_opcode_handler);
    idt.device_not_available = idt_entry::create(_device_not_available_handler);
    idt.double_fault = idt_entry::create(_double_fault_handler);
    idt.invalid_tss = idt_entry::create(_invalid_tss_handler);
    idt.coprocessor_seg_overrun = idt_entry::create(_coprocessor_seg_overrun_handler);
    idt.segment_not_present = idt_entry::create(_segment_not_present_handler);
    idt.stack_segment_fault = idt_entry::create(_stack_segment_fault_handler);
    idt.general_protection_fault = idt_entry::create(_general_protection_fault_handler);
    idt.page_fault = idt_entry::create(_page_fault_handler);
    idt.x87_fpu_fault = idt_entry::create(_x87_fpu_fault_handler);
    idt.alignment_check = idt_entry::create(_alignment_check_handler);
    idt.machine_check = idt_entry::create(_machine_check_handler);
    idt.simd_fpu_fault = idt_entry::create(_simd_fpu_fault_handler);
    idt.virt = idt_entry::create(_virt_handler);
    idt.security_exception = idt_entry::create(_security_exception_handler);

    idt.rest[11] = idt_entry::create(_irq0_handler);
    return idt;
}
}