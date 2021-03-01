.extern syscall_entry
.global raw_syscall_entry

raw_syscall_entry:
    pushq %r11
    pushq %rcx

    pushq %rdi
    pushq %rsi

    mov %rsp, %rdi
    callq syscall_entry

    popq %rsi
    popq %rdi

    popq %rcx
    popq %r11
    sysretq