/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // set stack before our code
    ldr     x1, =_start

    // set up EL1
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // running at EL3?
    cmp     x0, #12
    bne     5f
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 5f
    msr     elr_el3, x2
    eret

    // running at EL2?
5:
    cmp     x0, #4
    beq     5f
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    mov	x0, #0x33ff
    msr	cptr_el2, x0	// Disable coprocessor traps to EL2
    msr	hstr_el2, xzr	// Disable coprocessor traps to EL2
    mov	x0, #3 << 20
    msr cpacr_el1, x0 // Enable FP/SIMD at EL1

    // enable AArch64 in EL1
	mov	x0, #(1 << 31)						// 64bit EL1
	msr	hcr_el2, x0

	mov	x0, #0x0800
	movk	x0, #0x30d0, lsl #16
	orr    x0, x0, #(0x1 << 2)            // The C bit on (data cache).
	orr    x0, x0, #(0x1 << 12)           // The I bit on (instruction cache)
	msr	sctlr_el1, x0

    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2

    // set up exception handlers
    ldr     x2, =_vectors
    msr     vbar_el2, x2
    ldr     x2, =_vectors
    msr     vbar_el1, x2

    mov     x2, #0x40000
    msr     sp_el1, x2

    eret

5:  ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    mrs		x1, sctlr_el1
    mov     x2, #(1 << 1 | 1 << 3 | 1 << 4)
	bic     x1, x1, x2  // clear SA0, SA and A bits
	msr     sctlr_el1, x1
	isb

	msr     spsel, #1
	isb

    // jump to C code, should not return
4:  bl      kernel_main
    // for failsafe, halt this core too
    b       1b

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
	str	x30, [sp, #16 * 15]
	.endm

	.macro	kernel_exit
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
	ldr	x30, [sp, #16 * 15]
	add	sp, sp, #S_FRAME_SIZE
	.endm


.extern exc_handler
.section ".text.vectors"
_handlers:
// synchronous
_sync_vec:
    .align  7
    kernel_entry
    mov     x0, #0
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      exc_handler
    kernel_exit
    eret

_irq_vec:
    // IRQ
    .align  7
    kernel_entry
    mov     x0, #1
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      exc_handler
    kernel_exit
    eret

_fiq_vec:
    // FIQ
    .align  7
    kernel_entry
    mov     x0, #2
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      exc_handler
    kernel_exit
    eret

_serror_vec:
    // SError
    .align  7
    kernel_entry
    mov     x0, #3
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      exc_handler
    kernel_exit
    eret


.align 11
_vectors:
    // synchronous
    .align  7
    b _sync_vec

    // IRQ
    .align  7
    b _irq_vec

    // FIQ
    .align  7
    b _fiq_vec

    // SError
    .align  7
    b _serror_vec

    // synchronous
    .align  7
    b _sync_vec

    // IRQ
    .align  7
    b _irq_vec

    // FIQ
    .align  7
    b _fiq_vec

    // SError
    .align  7
    b _serror_vec