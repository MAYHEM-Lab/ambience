project(tos)

set(TOS_CORE_HEADERS 
	tos/atomic.hpp
	tos/ft.hpp
	tos/intrusive_list.hpp
	tos/scheduler.hpp
	tos/semaphore.hpp
	tos/thread_info.hpp
    tos/waitable.hpp
	tos/devices.hpp 
	tos/promise.hpp 
	tos/chrono.hpp 
	tos/mutex.hpp
	tos/intrusive_ptr.hpp 
	tos/char_stream.hpp 
	tos/utility.hpp 
	tos/arch.hpp 
	tos/type_traits.hpp
		tos/event.hpp)

set(TOS_CORE_SRCS
	ft.cpp
	semaphore.cpp
	arch.cpp)

add_library(tos_print tos/print.hpp print.cpp)
target_compile_options(tos_print PUBLIC -std=gnu++14)
target_include_directories(tos_print PUBLIC ".")

if (ARDUINO_FOUND)
    generate_arduino_library(tos_core
            SRCS ${TOS_CORE_SRCS}
            HDRS ${TOS_CORE_HEADERS}
            BOARD nano)
else()
    add_library(tos_core ${TOS_CORE_SRCS} ${TOS_CORE_HEADERS})
endif()

#target_compile_features(tos_core PUBLIC cxx_std_14)
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra -std=gnu++14)
target_include_directories(tos_core PUBLIC ".")

add_subdirectory(drivers)

if (TOS_ARDUINO)
	add_subdirectory(arch/arduino)
	target_link_libraries(tos_core arduino_impl)
	target_link_libraries(tos_core avr_gpio)
elseif(TOS_AVR)
	add_subdirectory(arch/avr)
	target_link_libraries(tos_core PUBLIC avr_impl)
elseif(TOS_X86)
	add_subdirectory(arch/x86)
	target_link_libraries(tos_core PUBLIC x86_impl)
elseif(TOS_ESP)
	add_subdirectory(arch/lx106)
	target_link_libraries(tos_core PUBLIC lx106_impl)
endif()