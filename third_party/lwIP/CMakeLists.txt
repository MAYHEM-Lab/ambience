set(LWIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/src/include)
add_subdirectory(src)

add_library(lwip_support "" include/tos/lwip/udp.hpp include/tos/lwip/utility.hpp include/tos/lwip/common.hpp include/tos/lwip/tcp.hpp)
target_sources(lwip_support PRIVATE
    support.cpp
    include/lwipopts.h
    include/arch/cc.h)
target_include_directories(lwip_support PUBLIC include)
target_include_directories(lwip_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/src/include)
target_link_libraries(lwip_support PUBLIC tos::drivers)

target_link_libraries(lwipcore PUBLIC lwip_support)

if (TARGET hosted_tap_network)
    add_executable(lwip_builds build.cpp)
    target_link_libraries(lwip_builds PUBLIC tos_core arch_drivers lwipcore tos_dynamic_log)
endif()