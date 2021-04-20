project(tos CXX ASM)

add_library(arch_x86_64
    include/tos/x86_64/spmanip.hpp 
    arch.cpp 
    include/tos/x86_64/assembly.hpp 
    include/tos/x86_64/interrupts.hpp 
    include/tos/x86_64/idt.hpp 
    include/tos/x86_64/port.hpp 
    include/tos/x86_64/mmu.hpp 
    mmu.cpp 
    include/tos/x86_64/gdt.hpp 
    include/tos/x86_64/exception.hpp 
    exception.cpp 
    idt.cpp 
    include/tos/x86_64/pic.hpp
    include/tos/x86_64/pci.hpp
    pci.cpp
    include/tos/x86_64/tss.hpp
    tss.cpp
    include/tos/x86_64/msr.hpp
    syscall_entry.S
    include/tos/x86_64/syscall.hpp
    syscall.cpp
    include/tos/x86_64/cpuid.hpp
    cpuid.cpp
    include/tos/x86_64/address_space.hpp
    address_space.cpp
    include/tos/x86_64/backtrace.hpp
    backtrace.cpp
)
target_include_directories(arch_x86_64 PUBLIC include)
target_link_libraries(arch_x86_64 PUBLIC tos_util_core tos_pci tos::libcxx tos_address_space)
target_link_libraries(arch_x86_64 PRIVATE tos_arch_core)
target_compile_features(arch_x86_64 PUBLIC cxx_std_20)

set(TOS_ARCH_HAVE_ADDRESS_SPACE ON CACHE BOOL "Have adress space")