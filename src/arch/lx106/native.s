#define CAUSE_LOADSTORE 3

#.global tos_set_stack_ptr
#tos_set_stack_ptr:
#    mov a3, a0 # ret addr in a0
#    movi a0, 0
#    mov sp, a2
#    jx a3

#.global call_user_start
#call_user_start:
#    j call_user_start_sdk

/*.section .MyUserExceptionVector.text
.global MyUserExceptionVector
MyUserExceptionVector:
    j _UserExceptionVector

    wsr     a1, excsave1
    rsr     a1, exccause
    beqi    a1, 3, LoadStoreErrorHandler
    j       UserExceptionHandler

.section .vecbase.text, "x"
LoadStoreErrorHandler:
    jx a0

.global UserExceptionHandler
UserExceptionHandler:
    j _UserExceptionVector*/
