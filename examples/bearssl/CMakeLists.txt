
add_executable(bear main.cpp)
target_link_libraries(bear PUBLIC tos::core tos::print)


if (TOS_ESP)
    target_link_libraries(bear PUBLIC esp82_drivers)
    target_include_directories(bear PUBLIC "/home/fatih/bearssl-esp8266/inc")
    target_link_libraries(bear PUBLIC /home/fatih/bearssl-esp8266/esp8266/libbearssl.a)
elseif(TOS_X86)
    target_link_libraries(bear PUBLIC x86_drivers)
    target_include_directories(bear PUBLIC "/home/fatih/BearSSL/inc")
    target_link_libraries(bear PUBLIC /home/fatih/BearSSL/build/libbearssl.a)
endif()