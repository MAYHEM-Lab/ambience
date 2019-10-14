include(CMakeRC)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_library(tos_mem include/tos/memory.hpp src/memory.cpp)

set(TOS_CORE_HEADERS
        include/tos/devices.hpp
        include/tos/char_stream.hpp
        include/tos/arch.hpp
        include/tos/version.hpp
        include/tos/debug/debug.hpp
        include/tos/device_id.hpp
        include/tos/driver_traits.hpp
        include/tos/delay.hpp
        include/tos/barrier.hpp
        include/tos/debug/stack_dump.hpp 
        include/tos/debug/panic.hpp)

add_library(tos_print include/tos/print.hpp src/print.cpp)
target_link_libraries(tos_print PUBLIC tos_util tos::libcxx)
add_library(tos::print ALIAS tos_print)

add_library(tos_core src/stub.cpp ${TOS_CORE_HEADERS} src/tos_init.cpp src/device_id.cpp src/debug.cpp)
target_compile_definitions(tos_core PUBLIC TOS_GIT_SHA1=\"${GIT_SHA1}\")
target_link_libraries(tos_core PUBLIC tos_mem tos_util)
target_link_libraries(tos_core PUBLIC tos::libcxx)
add_library(tos::core ALIAS tos_core)

add_library(tos_arch_core src/stub.cpp)
target_link_libraries(tos_arch_core PUBLIC ${TOS_ARCH_NAME}_core)

add_library(tos_arch include/tos/arch.hpp src/stub.cpp)
target_link_libraries(tos_arch PUBLIC ${TOS_ARCH_NAME}_impl)
message(STATUS ${TOS_ARCH_NAME}_impl)

add_library(tos_interrupt include/tos/interrupt.hpp src/interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch_core)

target_link_libraries(tos_core PUBLIC tos_ft tos_arch_core tos_interrupt tos_arch)
target_compile_definitions(tos_core PUBLIC TOS_ARCH_${TOS_ARCH_NAME})

tos_install(tos_arch ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_arch_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_interrupt ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_print ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_mem ${CMAKE_CURRENT_SOURCE_DIR}/include)
