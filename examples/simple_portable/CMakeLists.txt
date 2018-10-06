project(tos)

add_executable(simple_portable main.cpp)
target_link_libraries(simple_portable tos_core tos_print tos_tty)

if(TOS_ARM)
    target_link_libraries(simple_portable nrf52_drivers)
elseif(TOS_AVR)
    target_link_libraries(simple_portable avr_drivers)
elseif(TOS_ESP)
    target_link_libraries(simple_portable esp82_drivers)
else()# (TOS_X86)
    target_link_libraries(simple_portable x86_drivers)
endif()
