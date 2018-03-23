.global _tos_set_stack_ptr
_tos_set_stack_ptr:
    popq %rax # remember the return value
    movq %rdi, %rsp # change the stack pointer
    pushq %rax
    ret
