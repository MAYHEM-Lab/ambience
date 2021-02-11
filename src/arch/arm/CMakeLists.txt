add_library(arch_arm "" include/tos/arm/processor_state.hpp)
target_include_directories(arch_arm PUBLIC include)
target_sources(arch_arm PRIVATE
        include/tos/arm/cmsis.hpp
        include/tos/arm/assembly.hpp
        include/tos/arm/nvic.hpp
        include/tos/arm/mpu.hpp
        include/tos/arm/core.hpp
        include/tos/arm/spmanip.hpp
        include/tos/arm/interrupts.hpp
        include/tos/arm/exception.hpp
        stub.cpp
        mpu.cpp
        core.cpp
        nvic.cpp
        exception.cpp
)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers tos_mmio tos::libcxx)
