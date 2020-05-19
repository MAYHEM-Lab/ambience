add_library(tos_sntp tos_sntp.cpp include/tos/sntp.hpp)
target_link_libraries(tos_sntp PUBLIC tos_core)
target_include_directories(tos_sntp PUBLIC include)

if (NOT ${TOS_PLATFORM} MATCHES "esp82")
    return()
endif()

add_library(lwip_sntp sntp.cpp include/lwip_sntp/sntp.h include/lwip_sntp/time.h include/tos/sntp.hpp)
#target_compile_definitions(lwip_sntp PUBLIC LWIP_DEBUG)
target_link_libraries(lwip_sntp PUBLIC lwip tos_core)
target_include_directories(lwip_sntp PUBLIC "include")
