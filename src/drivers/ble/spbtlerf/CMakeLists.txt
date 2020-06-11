if (NOT ${TOS_PLATFORM} MATCHES "stm32")
    return()
endif ()

add_subdirectory(spbtle-rf)

add_library(tos_spbtlerf
        include/tos/device/spbtlerf/common.hpp
        include/tos/device/spbtlerf/adapter.hpp
        src/gatt.cpp
        src/adapter.cpp
        src/driver_comm.cpp include/tos/device/spbtlerf/events.hpp src/events.cpp)
target_include_directories(tos_spbtlerf PUBLIC include)
target_include_directories(tos_spbtlerf PUBLIC spbtle-rf)
target_link_libraries(tos_spbtlerf PUBLIC spbtle-rf tos_util_core tos_core tos_ft)