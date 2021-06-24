project(tos C CXX ASM)

add_library(ambience_user_core platform.cpp include/tos/platform.hpp)
target_include_directories(ambience_user_core PUBLIC include)

add_library(ambience_user_impl impl.cpp)
target_link_libraries(ambience_user_impl PUBLIC ambience_user_core)
