.global tos_call_trampoline

tos_call_trampoline:
    movq %r15, %rdi
    jmpq *%r14
