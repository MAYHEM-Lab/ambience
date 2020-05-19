if (NOT ${TOS_PLATFORM} MATCHES "esp82")
    return()
endif()

add_library(esp_sdk INTERFACE)
target_include_directories(esp_sdk SYSTEM INTERFACE "src/include")
target_include_directories(esp_sdk SYSTEM INTERFACE "src/third_party/include")
target_link_libraries(esp_sdk INTERFACE "-L${CMAKE_CURRENT_SOURCE_DIR}/src/lib")
target_link_libraries(esp_sdk INTERFACE "-L${CMAKE_CURRENT_SOURCE_DIR}/src/ld")
