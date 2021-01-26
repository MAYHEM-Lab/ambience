add_library(arch_aarch64
        aarch64.cpp
        mmu.cpp
        include/tos/aarch64/mmu.hpp
        include/tos/aarch64/spmanip.hpp
        include/tos/aarch64/interrupts.hpp
        include/tos/aarch64/exception.hpp
        exception.cpp
        include/tos/aarch64/semihosting.hpp
        semihosting.cpp
)
target_include_directories(arch_aarch64 PUBLIC include)
target_compile_features(arch_aarch64 PUBLIC cxx_std_20)
target_link_libraries(arch_aarch64 PUBLIC tos_util_core tos_util tos::libcxx)