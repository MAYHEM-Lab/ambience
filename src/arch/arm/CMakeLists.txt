add_library(arch_arm "" include/arch/assembly.hpp)
target_include_directories(arch_arm PUBLIC include)
target_sources(arch_arm PRIVATE
        include/arch/mpu.hpp
        stub.cpp
        mpu.cpp)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers)