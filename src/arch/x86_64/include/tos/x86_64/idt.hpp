#pragma once

#include <tos/expected.hpp>

namespace tos::x86_64 {
enum class idt_error {

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

template <class HandlerT>
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
    idt_entry<interrupt_handler_t> div_by_zero;
    idt_entry<interrupt_handler_t> debug;
    idt_entry<interrupt_handler_t> nmi;
    idt_entry<interrupt_handler_t> breakpoint;
    idt_entry<interrupt_handler_t> overflow;
    idt_entry<interrupt_handler_t> out_of_bounds;
    idt_entry<interrupt_handler_t> invalid_opcode;
    idt_entry<interrupt_handler_t> device_not_available;
    idt_entry<exception_handler_t> double_fault;
    idt_entry<exception_handler_t> invalid_tss;
    idt_entry<exception_handler_t> segment_not_present;
    idt_entry<exception_handler_t> stack_segment_fault;
    idt_entry<exception_handler_t> general_protection_fault;
    idt_entry<interrupt_handler_t> page_fault;
    idt_entry<interrupt_handler_t> x87_fpu_fault;
    idt_entry<interrupt_handler_t> alignment_check;
    idt_entry<interrupt_handler_t> machine_check;
    idt_entry<interrupt_handler_t> simd_fpu_fault;
    idt_entry<interrupt_handler_t> virt;
    idt_entry<interrupt_handler_t> security_exception;
    idt_entry<interrupt_handler_t> rest[256 - 20];
};

static_assert(sizeof (interrupt_descriptor_table) == 256 * 16);
}