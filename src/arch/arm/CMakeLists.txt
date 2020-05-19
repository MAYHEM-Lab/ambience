add_library(arch_arm "")
target_include_directories(arch_arm PUBLIC include)
target_sources(arch_arm PRIVATE
        include/arch/cmsis.hpp
        include/arch/assembly.hpp
        include/arch/nvic.hpp
        include/arch/mpu.hpp
        include/arch/core.hpp
        stub.cpp
        mpu.cpp
        core.cpp)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers tos_mmio)
