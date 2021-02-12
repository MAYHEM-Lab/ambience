target_sources(picolibc
        PRIVATE
        strcmp.S
        aeabi_memcpy.c
        memchr.S
        strcpy.c
        setjmp.S
        aeabi_memclr.c
        aeabi_memmove.c
        aeabi_memset-soft.S
        strlen-stub.c
        memcpy-stub.c
        strlen.S
        memcpy.S)

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv7")
    target_compile_definitions(picolibc PUBLIC __ARM_ARCH_7EM__)
endif()

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv6")
    target_compile_definitions(picolibc PUBLIC __ARM_ARCH_6M__)
endif()