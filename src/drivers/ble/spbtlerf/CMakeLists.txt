if (NOT ${TOS_PLATFORM} MATCHES "stm32")
    return()
endif ()

add_library(tos_spbtlerf
        include/tos/device/spbtlerf/common.hpp
        include/tos/device/spbtlerf/adapter.hpp
        src/gatt.cpp
        src/adapter.cpp
        include/tos/device/spbtlerf/events.hpp
        src/events.cpp)
target_include_directories(tos_spbtlerf PUBLIC include)
target_link_libraries(tos_spbtlerf PRIVATE bluenrgms)
target_link_libraries(tos_spbtlerf PUBLIC tos_util_core arch_drivers tos_core tos_ft)