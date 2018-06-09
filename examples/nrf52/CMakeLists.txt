add_executable(nrf52_blink main.cpp)
target_link_libraries(nrf52_blink PUBLIC tos_core tos_print nrf52_drivers)

add_custom_command(
    TARGET nrf52_blink POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ./nrf52_blink${CMAKE_EXECUTABLE_SUFFIX} -O binary ./app.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Convert to binary image"
)

add_custom_command(
    TARGET nrf52_blink POST_BUILD
    COMMAND size ./nrf52_blink${CMAKE_EXECUTABLE_SUFFIX}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Calculate size"
)