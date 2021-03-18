#include "state.S"

.extern syscall_entry
.global raw_syscall_entry

/*
 * This entry/exit routine allows us to have identical return paths for both syscalls and exceptions.
 * Therefore, we can take a syscall, and resume another thread that was preempted via a timer interrupt and vice versa.
 */

raw_syscall_entry:
    cli
    // Values pushed here are not really correct!
    // SS and CS are just garbage.
    // RSP gets -8 due to the first SS push.
    // They get handled in the syscall_entry.
    push %rsp   // SS
    push %rsp   // RSP
    push %r11   // FLAGS
    push %rcx   // CS
    push %rcx   // RIP

    push %rax   // EC, we don't care that it's rax

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

    mov %rsp, %rdi
    callq syscall_entry

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

    add $8, %rsp

    iretq
    #sysretq