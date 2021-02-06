target_sources(picolibc
        PRIVATE
        strcmp.S
        strlen-armv7.S
        memcpy-armv7m.S
        aeabi_memcpy.c
        memchr.S
        strcpy.c
        setjmp.S
        aeabi_memclr.c
        aeabi_memmove.c
        aeabi_memset-soft.S
        strlen-stub.c)
target_compile_definitions(picolibc PUBLIC __ARM_ARCH_7EM__)