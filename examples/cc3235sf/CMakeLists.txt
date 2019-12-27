add_library(cc32xx_wlan wlan.cpp include/arch/wlan.hpp include/arch/detail/sock_rt.hpp include/arch/detail/events.hpp include/arch/tcp.hpp include/arch/udp.hpp include/arch/detail/socket_base.hpp include/arch/detail/errors.hpp socket_base.cpp include/arch/detail/fwd.hpp socket_runtime.cpp udp_socket.cpp tcp.cpp)
target_link_libraries(cc32xx_wlan PUBLIC tos_core arch_drivers ti_simplelink_src)
target_include_directories(cc32xx_wlan PUBLIC include)
set_target_properties(cc32xx_wlan PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(cc32xx_wlan PUBLIC "-Wl,--undefined=SimpleLinkSocketTriggerEventHandler")

add_executable(cc3235_demo main.cpp)
target_link_libraries(cc3235_demo PUBLIC tos_core arch_drivers cc32xx_wlan)
set_target_properties(cc3235_demo PROPERTIES CXX_EXTENSIONS OFF)