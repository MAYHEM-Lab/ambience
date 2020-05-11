set(LWIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/src/include)
add_subdirectory(src)

add_library(lwip_support "")
target_sources(lwip_support PRIVATE
    support.cpp
    include/lwipopts.h
    include/arch/cc.h)
target_include_directories(lwip_support PUBLIC include)

target_link_libraries(lwipcore PUBLIC lwip_support)