add_library(arch_arm "" include/tos/arm/spmanip.hpp include/tos/arm/interrupts.hpp)
target_include_directories(arch_arm PUBLIC include)
target_sources(arch_arm PRIVATE
        include/tos/arm/cmsis.hpp
        include/tos/arm/assembly.hpp
        include/tos/arm/nvic.hpp
        include/tos/arm/mpu.hpp
        include/tos/arm/core.hpp
        stub.cpp
        mpu.cpp
        core.cpp
        nvic.cpp)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers tos_mmio)
