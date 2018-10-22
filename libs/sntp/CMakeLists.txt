
add_library(lwip_sntp sntp.cpp include/lwip_sntp/sntp.h include/lwip_sntp/time.h)
target_compile_definitions(lwip_sntp PUBLIC LWIP_DEBUG)
target_link_libraries(lwip_sntp PUBLIC lwip tos_core)
target_include_directories(lwip_sntp PUBLIC "include")