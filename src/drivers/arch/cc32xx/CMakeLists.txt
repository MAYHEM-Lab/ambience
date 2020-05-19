add_library(cc32xx_timer include/arch/timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(cc32xx_timer PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_timer PUBLIC cxx_std_17)
target_include_directories(cc32xx_timer PUBLIC include)

add_library(cc32xx_uart include/arch/uart.hpp src/uart.cpp)
target_link_libraries(cc32xx_uart PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_uart PUBLIC cxx_std_17)
target_include_directories(cc32xx_uart PUBLIC include)

add_library(cc32xx_gpio include/arch/gpio.hpp src/stub.cpp)
target_link_libraries(cc32xx_gpio PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_gpio PUBLIC cxx_std_17)
target_include_directories(cc32xx_gpio PUBLIC include)

add_library(cc32xx_i2c include/arch/i2c.hpp src/stub.cpp)
target_link_libraries(cc32xx_i2c PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_i2c PUBLIC cxx_std_17)
target_include_directories(cc32xx_i2c PUBLIC include)

add_library(cc32xx_flash include/arch/flash.hpp src/flash.cpp)
target_link_libraries(cc32xx_flash PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_flash PUBLIC cxx_std_17)
target_include_directories(cc32xx_flash PUBLIC include)

add_library(cc32xx_wlan
        include/arch/wlan.hpp
        include/arch/detail/sock_rt.hpp
        include/arch/detail/events.hpp
        include/arch/tcp.hpp
        include/arch/udp.hpp
        include/arch/detail/socket_base.hpp
        include/arch/detail/errors.hpp
        include/arch/detail/fwd.hpp
        src/wlan.cpp
        src/socket_base.cpp
        src/socket_runtime.cpp
        src/udp_socket.cpp
        src/tcp.cpp src/wlan_events.cpp src/shared_variables.hpp)

target_link_libraries(cc32xx_wlan PUBLIC tos_core arch_drivers ti_simplelink_src)
target_include_directories(cc32xx_wlan PUBLIC include)
set_target_properties(cc32xx_wlan PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(cc32xx_wlan PUBLIC "-Wl,--undefined=SimpleLinkSocketTriggerEventHandler")

add_library(cc32xx_crypto
        include/arch/crypto/aes.hpp
        include/arch/crypto/sha2.hpp
        src/stub.cpp include/arch/crypto/crypto.hpp src/crypto/aes.cpp)
target_link_libraries(cc32xx_crypto PUBLIC ti_drivers caps)
target_include_directories(cc32xx_crypto PUBLIC include)

add_library(cc32xx_drivers INTERFACE)
target_link_libraries(cc32xx_drivers
        INTERFACE
        cc32xx_timer
        cc32xx_uart
        cc32xx_gpio
        cc32xx_i2c
        cc32xx_wlan
        cc32xx_crypto
        cc32xx_flash
        )

add_library(arch_drivers ALIAS cc32xx_drivers)