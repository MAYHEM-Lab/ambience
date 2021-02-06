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
)
target_include_directories(arch_x86_64 PUBLIC include)
target_link_libraries(arch_x86_64 PUBLIC tos_util_core tos_util tos::libcxx)
target_compile_features(arch_x86_64 PUBLIC cxx_std_20)