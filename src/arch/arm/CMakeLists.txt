enable_language(ASM)

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
        startup_common.cpp
        include/tos/arm/startup_common.hpp
        call_trampoline.S
)
target_link_libraries(arch_arm PUBLIC 
    tos_util_core cmsis::core tos_drivers tos_mmio tos::libcxx tos_address_space tos_mem
    quikshare
)
target_compile_features(arch_arm PUBLIC cxx_std_20)

target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2iz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_dmul") 
target_link_libraries(arch_arm PUBLIC "-u __udivmoddi4") 
target_link_libraries(arch_arm PUBLIC "-u __divmoddi4") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_dadd") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2lz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_l2d") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_dsub") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2uiz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_ui2d") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_i2d") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_ddiv") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_f2d") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2f") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_ul2f") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_ul2d") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_l2f") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2ulz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_f2ulz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_f2lz") 
target_link_libraries(arch_arm PUBLIC "-u __aeabi_dcmpun")

# target_link_libraries(arch_arm PUBLIC "-u __udivmoddi4")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2iz")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_dmul")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_ui2d")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_dadd")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_i2d")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_ddiv")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_dsub")
# target_link_libraries(arch_arm PUBLIC "-u __aeabi_d2f")


if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv7e")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv7em)
elseif (${CMAKE_C_COMPILER_TARGET} MATCHES "armv7")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv7m)
endif()

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv6")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv6m)
endif()

set(TOS_ARCH_HAVE_ADDRESS_SPACE ON CACHE BOOL "Have adress space")