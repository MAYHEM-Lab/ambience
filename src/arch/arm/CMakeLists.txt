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
)
target_link_libraries(arch_arm PUBLIC tos_util_core cmsis::core tos_drivers tos_mmio tos::libcxx tos_address_space)
target_compile_features(arch_arm PUBLIC cxx_std_20)

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv7")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv7m)
endif()

if (${CMAKE_C_COMPILER_TARGET} MATCHES "armv6")
    target_link_libraries(arch_arm PUBLIC clang_rt.builtins-armv6m)
endif()

set(TOS_ARCH_HAVE_ADDRESS_SPACE ON CACHE BOOL "Have adress space")