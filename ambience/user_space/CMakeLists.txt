add_library(ae_user_space
        user_space.cpp
        include/tos/ae/user_space.hpp
        entry.cpp
        syscall/include/tos/ae/detail/syscall.hpp)
target_link_libraries(ae_user_space PUBLIC tos_coro tos_util_core)
target_include_directories(ae_user_space PUBLIC include)

add_subdirectory(syscall)