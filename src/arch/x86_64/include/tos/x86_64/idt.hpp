#pragma once

#include <tos/expected.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::x86_64 {
enum class idt_error
{

};

struct [[gnu::packed]] idt_entry {
    uint16_t ptr_low;
    uint16_t gdt_sel;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t ptr_mid;
    uint32_t ptr_high;
    uint32_t __res;

    static idt_entry create(void (*handler)()) {
        idt_entry res{};
        auto ptr = reinterpret_cast<uint64_t>(handler);
        res.ptr_low = ptr & 0xFFFF;
        res.ptr_mid = (ptr >> 16) & 0xFFFF;
        res.ptr_high = (ptr >> 32) & 0xFFFFFFFF;
        res.ist = 0;
        res.type_attributes = 0x8e; // 0b1000'0xe type: 64-bit interrupt, in ring 0
        res.gdt_sel = 8;
        return res;
    }
};

struct [[gnu::packed]] interrupt_descriptor_table {
    idt_entry div_by_zero;              // 0x0
    idt_entry debug;                    // 0x1
    idt_entry nmi;                      // 0x2
    idt_entry breakpoint;               // 0x3
    idt_entry overflow;                 // 0x4
    idt_entry out_of_bounds;            // 0x5
    idt_entry invalid_opcode;           // 0x6
    idt_entry device_not_available;     // 0x7
    idt_entry double_fault;             // 0x8
    idt_entry coprocessor_seg_overrun;  // 0x9
    idt_entry invalid_tss;              // 0xA
    idt_entry segment_not_present;      // 0xB
    idt_entry stack_segment_fault;      // 0xC
    idt_entry general_protection_fault; // 0xD
    idt_entry page_fault;               // 0xE
    idt_entry x87_fpu_fault;            // 0xF
    idt_entry alignment_check;          // 0x10
    idt_entry machine_check;            // 0x11
    idt_entry simd_fpu_fault;           // 0x12
    idt_entry virt;                     // 0x13
    idt_entry security_exception;       // 0x14
    idt_entry rest[256 - 21];
};

static_assert(sizeof(interrupt_descriptor_table) == 256 * 16);

interrupt_descriptor_table default_idt();
} // namespace tos::x86_64

extern "C" {
void _div_by_zero_handler();
void _debug_handler();
void _nmi_handler();
void _breakpoint_handler();
void _overflow_handler();
void _out_of_bounds_handler();
void _invalid_opcode_handler();
void _device_not_available_handler();
void _double_fault_handler();
void _invalid_tss_handler();
void _coprocessor_seg_overrun_handler();
void _segment_not_present_handler();
void _stack_segment_fault_handler();
void _general_protection_fault_handler();
void _page_fault_handler();
void _x87_fpu_fault_handler();
void _alignment_check_handler();
void _machine_check_handler();
void _simd_fpu_fault_handler();
void _virt_handler();
void _security_exception_handler();
void _irq0_handler();
void _irq1_handler();
void _irq9_handler();
void _irq10_handler();
void _irq11_handler();
void _irq12_handler();
}