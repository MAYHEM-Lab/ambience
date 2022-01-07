.global tos_call_trampoline

.thumb
.thumb_func
tos_call_trampoline:
    mov r0, r5
    bx r4
