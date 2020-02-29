// AArch64 mode
 
.section ".text.boot"
 
.globl _start

_start:
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, run
hang: 
    wfe
    b       hang

run:
    ldr     x1, =_start
    
begin_exec:
    mov     sp, x1
    bl      kernel_main
    b hang
