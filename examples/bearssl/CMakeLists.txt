
add_executable(bear main.cpp keys.hpp)
target_link_libraries(bear PUBLIC tos::core tos::print caps)


if (${TOS_CORE} MATCHES "esp8266")
    target_link_libraries(bear PUBLIC esp82_drivers)
    target_include_directories(bear PUBLIC "/home/fatih/bearssl-esp8266/inc")
    target_link_libraries(bear PUBLIC /home/fatih/bearssl-esp8266/esp8266/libbearssl.a)
elseif(TOS_X86)
    target_link_libraries(bear PUBLIC x86_drivers)
    target_include_directories(bear PUBLIC "/home/fatih/BearSSL/inc")
    target_link_libraries(bear PUBLIC /home/fatih/BearSSL/build/libbearssl.a)
endif()