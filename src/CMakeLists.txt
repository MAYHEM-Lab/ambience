project(tos)

set(TOS_CORE_HEADERS
	tos/devices.hpp
	tos/char_stream.hpp
	tos/arch.hpp
		tos/compiler.hpp
    tos/version.hpp
	tos/debug.hpp
	tos/device_id.hpp
	tos/driver_traits.hpp
	tos/delay.hpp
    tos/barrier.hpp)

include(CMakeRC)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_subdirectory(core)

add_library(tos_print tos/print.hpp print.cpp)
target_link_libraries(tos_print PUBLIC tos_util tos::libcxx)
target_include_directories(tos_print PUBLIC ".")

add_library(tos_core stub.cpp ${TOS_CORE_HEADERS} tos_init.cpp device_id.cpp debug.cpp)
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra -std=gnu++14)
target_include_directories(tos_core PUBLIC ".")
target_compile_definitions(tos_core PUBLIC TOS_GIT_SHA1=\"${GIT_SHA1}\")
target_link_libraries(tos_core PUBLIC tos_mem)
target_link_libraries(tos_core PUBLIC tos::libcxx)

add_library(tos_ubsan ubsan.cpp)
target_link_libraries(tos_ubsan PUBLIC tos_core)

add_subdirectory(arch)

add_library(tos_arch tos/arch.hpp stub.cpp)
target_link_libraries(tos_arch PUBLIC ${TOS_ARCH_NAME}_impl)

add_library(tos_interrupt tos/interrupt.hpp interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch)
target_include_directories(tos_interrupt PUBLIC .)
target_link_libraries(tos_interrupt PUBLIC tos::libcxx)

target_link_libraries(tos_core PUBLIC tos_ft tos_arch tos_interrupt)
target_compile_definitions(tos_core PUBLIC TOS_ARCH_${TOS_ARCH_NAME})

message(STATUS "TOS_ARCH_${TOS_ARCH_NAME}")

add_subdirectory(util)
add_subdirectory(ft)
add_subdirectory(drivers)
