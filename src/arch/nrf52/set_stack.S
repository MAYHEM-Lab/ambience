.global _tos_set_stack_ptr
.global tos_set_stack_ptr
tos_set_stack_ptr:
_tos_set_stack_ptr:
    pop {r1, r2, r3}
    mov sp, r0
    push {r1, r2, r3}
    bx lr
