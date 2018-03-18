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
            SRCS ${TOS_CORE_SRCS} arduino_impl.cpp
            HDRS ${TOS_CORE_HDRS}
            BOARD uno)
	target_compile_options(tos_core PUBLIC -fno-threadsafe-statics -Wall -Wextra)
	target_compile_definitions(tos_core PUBLIC ARDUINO_CMAKE)
else()
    add_library(tos_core ${TOS_CORE_SRC} ${TOS_CORE_HDRS})
endif()

target_compile_features(tos_core PUBLIC cxx_std_14)
target_include_directories(tos_core PUBLIC ".")