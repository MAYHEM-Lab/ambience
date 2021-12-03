add_library(nrf52840_dk_board INTERFACE)
target_link_libraries(nrf52840_dk_board INTERFACE nrf52_drivers tos_bme280)
target_include_directories(nrf52840_dk_board INTERFACE "..")