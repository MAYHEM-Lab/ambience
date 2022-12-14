project(tos CXX ASM)

set(SDK_ROOT /opt/x-tools/tos-esp-sdk)
add_library(esp8266_core esp_core.cpp include/user_config.h native.s include/lx106_missing.hpp)
target_link_libraries(esp8266_core PUBLIC tos_util_core esp_sdk tos_arch_core)
tos_install(esp8266_core ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(esp8266_impl esp_impl.cpp)
tos_install(esp8266_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(esp8266_impl PUBLIC cxx_std_14)
target_link_libraries(esp8266_impl PUBLIC "-uuser_init -uespconn_init -ulwip_init -u__ubsan_handle_load_invalid_value")
target_link_libraries(esp8266_impl PUBLIC "-Tlinker.ld")
target_link_libraries(esp8266_impl PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(esp8266_impl PUBLIC tos_interrupt tos_jobs lwip esp_adapters esp_sdk)
target_compile_definitions(esp8266_impl PUBLIC LWIP_OPEN_SRC LWIP_RAW ESP8266 WITH_PGM_READ_HELPER)
target_compile_definitions(esp8266_impl PRIVATE SPI_FLASH_SIZE_MAP=4)

target_compile_options(esp8266_impl PUBLIC -Wno-literal-suffix)

add_library(esp_adapters esp_adapters.cpp)
target_link_libraries(esp_adapters PUBLIC tos_core tos_jobs)
target_link_libraries(esp_adapters PUBLIC "-Wl,--undefined=malloc -Wl,--undefined=free -Wl,--undefined=isxdigit")
tos_install(esp_adapters)

set_target_properties(esp8266_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)
target_link_libraries(esp8266_impl PUBLIC "-Wl,--start-group -lmain -lnet80211 -lwpa -lpp -lphy -lssl -lcrypto -Wl,--end-group -lgcc")
