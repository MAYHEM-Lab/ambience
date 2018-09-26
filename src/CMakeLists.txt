project(tos)

set(TOS_CORE_HEADERS
	tos/devices.hpp
	tos/char_stream.hpp
	tos/arch.hpp
	tos/init.hpp
	tos/compiler.hpp
	tos/chrono.hpp
	tos/version.hpp
	tos/debug.hpp
	tos/device_id.hpp
	tos/driver_traits.hpp
	tos/delay.hpp
)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_subdirectory(core)

add_library(tos_print tos/print.hpp print.cpp)
target_compile_options(tos_print PUBLIC -std=gnu++14)
target_link_libraries(tos_print PUBLIC tos_util)
target_include_directories(tos_print PUBLIC ".")

add_library(tos_core stub.cpp ${TOS_CORE_HEADERS} tos_init.cpp device_id.cpp debug.cpp)
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra -std=gnu++14)
target_include_directories(tos_core PUBLIC ".")
target_compile_definitions(tos_core PUBLIC TOS_GIT_SHA1=\"${GIT_SHA1}\")
target_link_libraries(tos_core PUBLIC tos_mem)

add_library(tos_ubsan ubsan.cpp)
target_link_libraries(tos_ubsan PUBLIC tos_core)

if(TOS_AVR)
	add_subdirectory(arch/avr)
	set(TOS_ARCH_NAME avr)
elseif(TOS_X86)
	add_subdirectory(arch/x86)
	set(TOS_ARCH_NAME x86)
elseif(TOS_ESP)
	add_subdirectory(arch/lx106)
	set(TOS_ARCH_NAME lx106)
elseif(TOS_ARM)
	add_subdirectory(arch/arm)
	set(TOS_ARCH_NAME arm)
endif()

add_library(tos_interrupt tos/interrupt.hpp interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch)
target_include_directories(tos_interrupt PUBLIC .)

add_library(tos_arch tos/arch.hpp stub.cpp)
target_link_libraries(tos_arch PUBLIC ${TOS_ARCH_NAME}_impl)

target_link_libraries(tos_core PUBLIC tos_ft tos_arch tos_interrupt)
target_compile_definitions(tos_core PUBLIC TOS_ARCH_${TOS_ARCH_NAME})
message(STATUS "TOS_ARCH_${TOS_ARCH_NAME}")

add_subdirectory(util)
add_subdirectory(ft)
add_subdirectory(drivers)
