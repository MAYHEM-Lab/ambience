.macro save_gpr ec:req
.ifeq \ec
    push %rax
.endif
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    sub $256, %rsp
    movdqu %xmm0, 0(%rsp)
    movdqu %xmm1, 16(%rsp)
    movdqu %xmm2, 32(%rsp)
    movdqu %xmm3, 48(%rsp)
    movdqu %xmm4, 64(%rsp)
    movdqu %xmm5, 80(%rsp)
    movdqu %xmm6, 96(%rsp)
    movdqu %xmm7, 112(%rsp)
    movdqu %xmm8, 128(%rsp)
    movdqu %xmm9, 144(%rsp)
    movdqu %xmm10, 160(%rsp)
    movdqu %xmm11, 176(%rsp)
    movdqu %xmm12, 192(%rsp)
    movdqu %xmm13, 208(%rsp)
    movdqu %xmm14, 224(%rsp)
    movdqu %xmm15, 240(%rsp)
.endm

.macro load_gpr
    movdqu 0(%rsp),  %xmm0
    movdqu 16(%rsp), %xmm1
    movdqu 32(%rsp), %xmm2
    movdqu 48(%rsp), %xmm3
    movdqu 64(%rsp), %xmm4
    movdqu 80(%rsp), %xmm5
    movdqu 96(%rsp), %xmm6
    movdqu 112(%rsp), %xmm7
    movdqu 128(%rsp), %xmm8
    movdqu 144(%rsp), %xmm9
    movdqu 160(%rsp), %xmm10
    movdqu 176(%rsp), %xmm11
    movdqu 192(%rsp), %xmm12
    movdqu 208(%rsp), %xmm13
    movdqu 224(%rsp), %xmm14
    movdqu 240(%rsp), %xmm15
    add $256, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    add $8, %rsp // This is either the error code or %rax, and we don't care about either
.endm

.macro make_isr actual_handler:req number:req ec:req
    .extern \actual_handler
    .global _\actual_handler
    _\actual_handler:
        save_gpr \ec
        mov %rsp, %rdi
        add $256, %rdi
        mov $\number, %rsi
        call \actual_handler
        load_gpr
        iretq
.endm

.extern irq_entry
.macro make_irq number:req
    .global _irq\number\()_handler
    _irq\number\()_handler:
        save_gpr 0
        mov %rsp, %rdi
        add $256, %rdi
        mov $(32 + \number), %rsi
        call irq_entry
        load_gpr
        iretq
    #make_isr irq\number\()_handler, (32 + \number), 0
.endm

make_isr div_by_zero_handler,               0,  0
make_isr debug_handler,                     1,  0
make_isr nmi_handler,                       2,  0
make_isr breakpoint_handler,                3,  0
make_isr overflow_handler,                  4,  0
make_isr out_of_bounds_handler,             5,  0
make_isr invalid_opcode_handler,            6,  0
make_isr device_not_available_handler,      7,  0
make_isr double_fault_handler,              8,  1
make_isr coprocessor_seg_overrun_handler,   9,  0
make_isr invalid_tss_handler,               10, 1
make_isr segment_not_present_handler,       11, 1
make_isr stack_segment_fault_handler,       12, 1
make_isr general_protection_fault_handler,  13, 1
make_isr page_fault_handler,                14, 1
make_isr x87_fpu_fault_handler,             15, 0
make_isr alignment_check_handler,           16, 1
make_isr machine_check_handler,             17, 1
make_isr simd_fpu_fault_handler,            18, 0
make_isr virt_handler,                      19, 0
make_isr security_exception_handler,        20, 0

make_irq 0
make_irq 1
make_irq 2
make_irq 3
make_irq 4
make_irq 5
make_irq 6
make_irq 7
make_irq 8
make_irq 9
make_irq 10
make_irq 11
make_irq 12
make_irq 13
make_irq 14
make_irq 15
