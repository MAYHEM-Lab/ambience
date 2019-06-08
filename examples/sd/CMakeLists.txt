project(tos)

add_executable(sdcard main.cpp)

target_link_libraries(sdcard PUBLIC tos::core tos_spi_sd arch_drivers tos::print)
