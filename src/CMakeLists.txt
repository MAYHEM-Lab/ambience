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
)

set(TOS_CORE_SRCS
	ft.cpp
	semaphore.cpp
	timer.cpp
	waitable.cpp
)

if (ARDUINO_FOUND)
    generate_arduino_library(tos_core
            SRCS ${TOS_CORE_SRCS}
            HDRS ${TOS_CORE_HDRS}
            BOARD uno)
else()
    add_library(tos_core ${TOS_CORE_SRCS} ${TOS_CORE_HDRS})
endif()

target_compile_features(tos_core PUBLIC cxx_std_14)
target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra)
target_include_directories(tos_core PUBLIC ".")

if (ARDUINO_FOUND)
	add_subdirectory(arch/arduino)
	target_link_libraries(tos_core arduino_impl)
else()
	add_subdirectory(arch/x86)
	target_link_libraries(tos_core PUBLIC x86_impl)
endif()