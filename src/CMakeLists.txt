project(tos)


set(TOS_CORE_HEADERS
	tos/devices.hpp
	tos/promise.hpp 
	tos/chrono.hpp 
	tos/char_stream.hpp
	tos/utility.hpp 
	tos/arch.hpp
	tos/type_traits.hpp tos/event.hpp)


add_library(tos_print tos/print.hpp print.cpp)
target_compile_options(tos_print PUBLIC -std=gnu++14)
target_include_directories(tos_print PUBLIC ".")

add_library(tos_core stub.cpp ${TOS_CORE_HEADERS})
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra -std=gnu++14)
target_include_directories(tos_core PUBLIC ".")

if (TOS_ARDUINO)
	add_subdirectory(arch/arduino)
	set(TOS_ARCH_NAME arduino)
elseif(TOS_AVR)
	add_subdirectory(arch/avr)
	set(TOS_ARCH_NAME avr)
elseif(TOS_X86)
	add_subdirectory(arch/x86)
	set(TOS_ARCH_NAME x86)
elseif(TOS_ESP)
	add_subdirectory(arch/lx106)
	set(TOS_ARCH_NAME lx106)
endif()

add_library(tos_interrupt tos/interrupt.hpp interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch)
target_include_directories(tos_interrupt PUBLIC .)

add_library(tos_arch tos/arch.hpp stub.cpp)
target_link_libraries(tos_arch PUBLIC ${TOS_ARCH_NAME}_impl)

target_link_libraries(tos_core PUBLIC tos_ft tos_arch tos_interrupt)

add_subdirectory(util)
add_subdirectory(ft)
add_subdirectory(drivers)
