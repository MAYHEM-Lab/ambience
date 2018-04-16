project(tos)

add_executable(sdcard main.cpp)

target_link_libraries(sdcard PUBLIC tos_core)