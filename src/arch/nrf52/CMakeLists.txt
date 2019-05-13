project(tos CXX ASM)

add_library(nrf52_core nrf52_core.cpp include/tos_arch.hpp)
tos_install(nrf52_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_core PUBLIC tos_util_core)

add_library(nrf52_impl nrf52_impl.cpp include/sdk_config.h startup.S ${SDK_ROOT}/modules/nrfx/mdk/system_nrf52.c)
tos_install(nrf52_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_impl PUBLIC nrf52_core)
target_compile_features(nrf52_impl PUBLIC cxx_std_17)

target_compile_definitions(nrf52_impl PUBLIC __HEAP_SIZE=8097 __STACK_SIZE=2048)

target_link_libraries(nrf52_impl PUBLIC tos_core)
target_link_libraries(nrf52_impl PUBLIC "--specs=nano.specs -lc -lnosys -lm")

target_link_libraries(nrf52_impl PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}/ld")
target_link_libraries(nrf52_impl PUBLIC "-T${TOS_LD_FILE}")

set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/${TOS_LD_FILE})
set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/nrf5x_common.ld)
