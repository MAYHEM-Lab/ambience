add_library(x86_stdio include/arch/stdio.hpp src/stdio.cpp)
target_link_libraries(x86_stdio PUBLIC tos_core tos_util)
tos_install(x86_stdio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_timer include/arch/timer.hpp src/stub.cpp)
target_link_libraries(x86_timer PUBLIC tos::core x86_hosted_impl)
tos_install(x86_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_tcp include/arch/tcp.hpp src/tcp.cpp)
target_link_libraries(x86_tcp PUBLIC tos::core x86_hosted_impl)
tos_install(x86_tcp ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_udp include/arch/udp.hpp src/udp.cpp)
target_link_libraries(x86_udp PUBLIC tos::core x86_hosted_impl)
tos_install(x86_udp ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_unix_sockets include/arch/unix_socket.hpp src/unix_socket.cpp)
target_link_libraries(x86_unix_sockets PUBLIC tos::core x86_hosted_impl)
tos_install(x86_unix_sockets ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(x86_usart PUBLIC tos::core x86_hosted_impl)
tos_install(x86_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(x86_drivers PUBLIC
        x86_stdio
        x86_timer
        x86_unix_sockets
        x86_usart
        x86_tcp
        x86_udp)
tos_install(x86_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(arch_drivers ALIAS x86_drivers)
