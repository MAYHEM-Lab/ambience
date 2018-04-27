.global tos_set_stack_ptr
tos_set_stack_ptr:
    mov a3, a0
    movi a0, 0
    mov sp, a2
    jx a3
