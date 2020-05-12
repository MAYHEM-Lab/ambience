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

add_library(x86_clocks include/arch/cpp_clocks.hpp src/cpp_clocks.cpp)
target_link_libraries(x86_clocks PUBLIC tos::core x86_hosted_impl)
tos_install(x86_clocks ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(x86_display
        include/arch/display.hpp)
target_include_directories(x86_display PUBLIC "include")
target_link_libraries(x86_display PUBLIC tos::drivers x86_hosted_impl tos_gfx)

add_library(hosted_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(hosted_drivers PUBLIC
        x86_stdio
        x86_timer
        x86_unix_sockets
        x86_usart
        x86_tcp
        x86_udp
        x86_clocks
        x86_display)
tos_install(hosted_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)


find_package(X11)

if (NOT DISABLE_X11 AND X11_FOUND)
    message(STATUS "Found X11, enabling display emulation")
    target_sources(x86_display PRIVATE
        src/x11_display.cpp)
    target_include_directories(x86_display PUBLIC ${X11_INCLUDE_DIR})
    target_link_libraries(x86_display PUBLIC ${X11_LIBRARIES})
else()
    target_sources(x86_display PRIVATE
            src/null_display.cpp)
endif()

add_library(arch_drivers ALIAS hosted_drivers)
add_subdirectory(docs)