#define S_FRAME_SIZE 272

.macro	kernel_entry
    sub	sp, sp, #S_FRAME_SIZE
    stp	x0, x1, [sp, #16 * 0]
    stp	x2, x3, [sp, #16 * 1]
    stp	x4, x5, [sp, #16 * 2]
    stp	x6, x7, [sp, #16 * 3]
    stp	x8, x9, [sp, #16 * 4]
    stp	x10, x11, [sp, #16 * 5]
    stp	x12, x13, [sp, #16 * 6]
    stp	x14, x15, [sp, #16 * 7]
    stp	x16, x17, [sp, #16 * 8]
    stp	x18, x19, [sp, #16 * 9]
    stp	x20, x21, [sp, #16 * 10]
    stp	x22, x23, [sp, #16 * 11]
    stp	x24, x25, [sp, #16 * 12]
    stp	x26, x27, [sp, #16 * 13]
    stp	x28, x29, [sp, #16 * 14]

    # These 3 registers are part of the exception handling state of AArch64.
    # Since we can take an exception from Thread A and return from an exception in Thread B,
    # without maintaining this global state in the stack, we would end up in the wrong place.
    mrs x21, sp_el0
    mrs x22, elr_el1
    mrs x23, spsr_el1

	stp	x30, x21, [sp, #16 * 15]
	stp	x22, x23, [sp, #16 * 16]
.endm

.macro	kernel_exit
	ldp	x22, x23, [sp, #16 * 16]
	ldp	x30, x21, [sp, #16 * 15]

	msr	sp_el0, x21
    msr	elr_el1, x22
    msr	spsr_el1, x23

    ldp	x0, x1, [sp, #16 * 0]
    ldp	x2, x3, [sp, #16 * 1]
    ldp	x4, x5, [sp, #16 * 2]
    ldp	x6, x7, [sp, #16 * 3]
    ldp	x8, x9, [sp, #16 * 4]
    ldp	x10, x11, [sp, #16 * 5]
    ldp	x12, x13, [sp, #16 * 6]
    ldp	x14, x15, [sp, #16 * 7]
    ldp	x16, x17, [sp, #16 * 8]
    ldp	x18, x19, [sp, #16 * 9]
    ldp	x20, x21, [sp, #16 * 10]
    ldp	x22, x23, [sp, #16 * 11]
    ldp	x24, x25, [sp, #16 * 12]
    ldp	x26, x27, [sp, #16 * 13]
    ldp	x28, x29, [sp, #16 * 14]
    add	sp, sp, #S_FRAME_SIZE
.endm

.extern sync_handler
.extern irq_handler
.extern fiq_handler
.extern serror_handler

.section ".text.vectors"

.extern _irq_count
.extern _irq_exit_count
_inc_atomic:
    ldaxr   w8, [x0]
    add     w8, w8, #1
    stlxr   w9, w8, [x0]
    cbnz    w9, _inc_atomic
    ret

_inc_normal:
    ldr     w8, [x0]
    add     w8, w8, #1
    str     w8, [x0]
    ret

_handlers:
_el0_sync_vec:
    kernel_entry
    mov     x0, 0
    mov     x1, sp
    bl      sync_handler
    kernel_exit
    eret

_el0_irq_vec:
    kernel_entry
    mov     x0, 0
    bl      irq_handler
    kernel_exit
    eret

_el1_sync_vec:
    kernel_entry
    mov     x0, 1
    mov     x1, sp
    bl      sync_handler
    kernel_exit
    eret

_el1_irq_vec:
    kernel_entry
    ldr     x0, =_irq_count
    bl      _inc_normal
    mov     x0, 1
    bl      irq_handler
    ldr     x0, =_irq_exit_count
    bl      _inc_normal
    kernel_exit
    eret

_el1_fiq_vec:
    kernel_entry
    mov     x0, 1
    bl      fiq_handler
    kernel_exit
    eret

_el1_serror_vec:
    kernel_entry
    mov     x0, 1
    bl      serror_handler
    kernel_exit
    eret

.global _vectors
.align 11
_vectors:
    // synchronous
    .align  7
    b _el1_sync_vec

    // IRQ
    .align  7
    b _el1_irq_vec

    // FIQ
    .align  7
    b _el1_fiq_vec

    // SError
    .align  7
    b _el1_serror_vec

    // synchronous
    .align  7
    b _el1_sync_vec

    // IRQ
    .align  7
    b _el1_irq_vec

    // FIQ
    .align  7
    b _el1_fiq_vec

    // SError
    .align  7
    b _el1_serror_vec

    // synchronous
    .align  7
    b _el0_sync_vec

    // IRQ
    .align  7
    b _el0_irq_vec

    // FIQ
    .align  7
    b _el1_fiq_vec

    // SError
    .align  7
    b _el1_serror_vec