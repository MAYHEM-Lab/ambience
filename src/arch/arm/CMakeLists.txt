add_library(arch_arm "")
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
        include/tos/arm/processor_state.hpp
        include/tos/arm/address_space.hpp
        address_space.cpp
        include/tos/arm/cyccnt_clock.hpp
)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers tos_mmio tos::libcxx tos_address_space)
target_compile_features(arch_arm PUBLIC cxx_std_20)
target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2iz -u __aeabi_dmul -u __udivmoddi4 -u __divmoddi4 -u __aeabi_dadd -u __aeabi_d2lz -u __aeabi_l2d -u __aeabi_dsub -u __aeabi_d2uiz -u __aeabi_ui2d -u __aeabi_i2d -u __aeabi_ddiv -u __aeabi_f2d -u __aeabi_d2f -u __aeabi_ul2f -u __aeabi_ul2d -u __aeabi_l2f -u __aeabi_d2ulz -u __aeabi_f2ulz -u __aeabi_f2lz -u __aeabi_dcmpun")

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv7")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv7m)
endif()

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv6")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv6m)
endif()

set(TOS_ARCH_HAVE_ADDRESS_SPACE ON CACHE BOOL "Have adress space")