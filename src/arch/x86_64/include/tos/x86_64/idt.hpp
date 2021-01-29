#pragma once

#include <tos/expected.hpp>

namespace tos::x86_64 {
enum class idt_error
{

};

struct interrupt_stack_frame_t {
    uintptr_t instr_ptr;
    uint64_t code_segment;
    uint64_t flags;
    uintptr_t stack_ptr;
    uint64_t stack_sgement;
};

using interrupt_handler_t = void (*)(interrupt_stack_frame_t*);
using exception_handler_t = void (*)(interrupt_stack_frame_t*, unsigned long int error);

template<class HandlerT>
struct [[gnu::packed]] idt_entry {
    uint16_t ptr_low;
    uint16_t gdt_sel;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t ptr_mid;
    uint32_t ptr_high;
    uint32_t __res;

    static expected<idt_entry, idt_error> create(HandlerT handler) {
        idt_entry res{};
        auto ptr = reinterpret_cast<uint64_t>(handler);
        res.ptr_low = ptr & 0xFFFF;
        res.ptr_mid = (ptr >> 16) & 0xFFFF;
        res.ptr_high = (ptr >> 32) & 0xFFFFFFFF;
        res.ist = 0;
        res.type_attributes = 0x8e;
        res.gdt_sel = 8;
        return res;
    }
};

struct [[gnu::packed]] interrupt_descriptor_table {
    idt_entry<interrupt_handler_t> div_by_zero;              // 0x0
    idt_entry<interrupt_handler_t> debug;                    // 0x1
    idt_entry<interrupt_handler_t> nmi;                      // 0x2
    idt_entry<interrupt_handler_t> breakpoint;               // 0x3
    idt_entry<interrupt_handler_t> overflow;                 // 0x4
    idt_entry<interrupt_handler_t> out_of_bounds;            // 0x5
    idt_entry<interrupt_handler_t> invalid_opcode;           // 0x6
    idt_entry<interrupt_handler_t> device_not_available;     // 0x7
    idt_entry<exception_handler_t> double_fault;             // 0x8
    idt_entry<exception_handler_t> invalid_tss;              // 0x9
    idt_entry<interrupt_handler_t> coprocessor_seg_overrun;  // 0xA
    idt_entry<exception_handler_t> segment_not_present;      // 0xB
    idt_entry<exception_handler_t> stack_segment_fault;      // 0xC
    idt_entry<exception_handler_t> general_protection_fault; // 0xD
    idt_entry<interrupt_handler_t> page_fault;               // 0xE
    idt_entry<interrupt_handler_t> x87_fpu_fault;            // 0xF
    idt_entry<interrupt_handler_t> alignment_check;          // 0x10
    idt_entry<interrupt_handler_t> machine_check;            // 0x11
    idt_entry<interrupt_handler_t> simd_fpu_fault;           // 0x12
    idt_entry<interrupt_handler_t> virt;                     // 0x13
    idt_entry<interrupt_handler_t> security_exception;       // 0x14
    idt_entry<interrupt_handler_t> rest[256 - 21];
};

static_assert(sizeof(interrupt_descriptor_table) == 256 * 16);
} // namespace tos::x86_64