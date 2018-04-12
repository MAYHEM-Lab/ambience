.global tos_set_stack_ptr
tos_set_stack_ptr:
    popq %rax # remember the return value
    movq %rdi, %rsp # change the stack pointer
    pushq %rax
    ret
