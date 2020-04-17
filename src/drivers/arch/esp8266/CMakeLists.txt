project(tos)

add_library(lx106_usart include/arch/usart.hpp src/uart_register.h src/new_uart_priv.h src/usart.cpp)
target_link_libraries(lx106_usart PRIVATE tos_core esp8266_impl)
tos_install(lx106_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(esp82_timer include/arch/timer.hpp src/timer.cpp)
tos_install(esp82_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(esp82_timer PUBLIC USE_US_TIMER) # without this, sdk only provides ms timers
target_link_libraries(esp82_timer PRIVATE tos_core esp8266_impl)

add_library(esp82_wifi include/arch/wifi.hpp src/wifi.cpp)
tos_install(esp82_wifi ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_wifi PUBLIC tos_core esp8266_impl)

add_library(esp82_udp include/arch/udp.hpp src/udp.cpp)
tos_install(esp82_udp ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_udp PUBLIC esp82_wifi tos_inet)

add_library(esp82_tcp include/arch/tcp.hpp src/tcp.cpp)
tos_install(esp82_tcp ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_tcp PUBLIC esp82_wifi tos_inet tos::libcxx)
target_compile_definitions(esp82_tcp PUBLIC ESP_TCP_VERBOSE)

add_library(esp82_gpio include/arch/gpio.hpp src/gpio.cpp)
tos_install(esp82_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_gpio PUBLIC tos_core tos_gpio)

add_library(esp82_i2c src/twim.cpp include/arch/twim.hpp src/si2c.cpp)
target_include_directories(esp82_i2c PRIVATE src)
tos_install(esp82_i2c ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_i2c PUBLIC esp82_gpio)

add_library(esp82_spi include/arch/spi.hpp src/spi.cpp src/spi.h src/spi_register.h)
target_include_directories(esp82_spi PRIVATE src)
tos_install(esp82_spi ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_spi PUBLIC esp82_gpio)

add_library(esp82_adc include/arch/adc.hpp src/adc.cpp)
target_include_directories(esp82_adc PRIVATE src)
tos_install(esp82_adc ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_adc PUBLIC tos_core)

add_library(esp82_nvm include/arch/nvm.hpp src/nvm.cpp)
tos_install(esp82_nvm ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esp82_nvm PUBLIC tos_core)

add_library(esp82_drivers src/stub.cpp include/arch/drivers.hpp)
target_link_libraries(esp82_drivers PUBLIC
        lx106_usart
        esp82_timer
        esp82_wifi
        esp82_tcp
        esp82_udp
        esp82_i2c
        esp82_nvm
        esp82_spi)
tos_install(esp82_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(esp82_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS esp82_drivers)