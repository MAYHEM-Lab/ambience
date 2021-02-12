project(tos CXX ASM)

add_library(nrf52_core nrf52_core.cpp)
tos_install(nrf52_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_core PUBLIC tos_util_core nrf5-sdk tos_arch_core)

add_library(nrf52_impl nrf52_impl.cpp startup.S isr_vector.cpp)
tos_install(nrf52_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_impl PUBLIC nrf52_core nrf5-sdk)
target_compile_features(nrf52_impl PUBLIC cxx_std_17)

target_compile_definitions(nrf52_impl PUBLIC __HEAP_SIZE=8097 __STACK_SIZE=2048)

target_link_libraries(nrf52_impl PUBLIC tos_jobs)

target_link_libraries(nrf52_impl PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}/ld")
target_link_libraries(nrf52_impl PUBLIC "-T${TOS_LD_FILE}")

set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/${TOS_LD_FILE})
set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/nrf5x_common.ld)
