add_library(stm32_impl main.cpp)

add_library(stm32_core include/tos_arch.hpp include/tos_platform.hpp stm32_core.cpp)
target_include_directories(stm32_core PUBLIC "/opt/x-tools/libopencm3/include")

target_link_libraries(stm32_core PUBLIC "-Tbluepill.ld")
target_link_libraries(stm32_core PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(stm32_core PUBLIC "-L/opt/x-tools/libopencm3/lib/")
target_link_libraries(stm32_core PUBLIC opencm3_stm32f1)
target_link_libraries(stm32_core PUBLIC "-specs=nano.specs -specs=nosys.specs -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -Wl,--undefined=main")
target_compile_features(stm32_core PUBLIC cxx_std_17)
target_link_libraries(stm32_core PUBLIC tos_util_core)

target_link_libraries(stm32_impl PUBLIC stm32_core)
target_link_libraries(stm32_impl PUBLIC tos_ft)

tos_install(stm32_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(stm32_core ${CMAKE_CURRENT_SOURCE_DIR}/include)