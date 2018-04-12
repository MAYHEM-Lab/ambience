project(tos C CXX)

set(TOS_CORE_HEADERS 
	tos/atomic.hpp
	tos/ft.hpp
	tos/intrusive_list.hpp
	tos/scheduler.hpp
	tos/semaphore.hpp
	tos/thread_info.hpp
	tos/timer.hpp
	tos/waitable.hpp
    tos/system.hpp
		tos/devices.hpp)

set(TOS_CORE_SRCS
	ft.cpp
	semaphore.cpp
	timer.cpp
	waitable.cpp tos/char_stream.hpp tos/print.hpp tos/utility.hpp tos/type_traits.hpp print.cpp tos/chrono.hpp tos/mutex.hpp tos/intrusive_ptr.hpp)

if (ARDUINO_FOUND)
    generate_arduino_library(tos_core
            SRCS ${TOS_CORE_SRCS}
            HDRS ${TOS_CORE_HEADERS}
            BOARD nano)
else()
    add_library(tos_core ${TOS_CORE_SRCS} ${TOS_CORE_HEADERS})
endif()

target_compile_features(tos_core PUBLIC cxx_std_14)
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra)
target_include_directories(tos_core PUBLIC ".")

if (TOS_ARDUINO)
	add_subdirectory(arch/arduino)
	add_subdirectory(drivers/arch/avr)
	target_link_libraries(tos_core arduino_impl)
	target_link_libraries(tos_core avr_gpio)
elseif(TOS_AVR)
	add_subdirectory(arch/avr)
	add_subdirectory(drivers/arch/avr)
	target_link_libraries(tos_core PUBLIC avr_impl)
	target_link_libraries(tos_core PUBLIC avr_gpio avr_usart)
else()
	add_subdirectory(arch/x86)
	target_link_libraries(tos_core PUBLIC x86_impl)
endif()