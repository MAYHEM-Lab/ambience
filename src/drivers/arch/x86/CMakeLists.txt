
add_library(x86_stdio include/arch/stdio.hpp src/stdio.cpp)
target_compile_options(x86_stdio PUBLIC -std=c++14)
target_link_libraries(x86_stdio PUBLIC tos_core tos_util)
tos_install(x86_stdio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_timer include/arch/timer.hpp src/stub.cpp)
target_link_libraries(x86_timer PUBLIC tos::core x86_impl)
tos_install(x86_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_tcp include/arch/tcp.hpp src/stub.cpp)
target_link_libraries(x86_tcp PUBLIC tos::core x86_impl)
tos_install(x86_tcp ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(x86_drivers PUBLIC x86_stdio x86_timer x86_tcp)
tos_install(x86_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)