add_library(nrf52_prs INTERFACE)
target_link_libraries(nrf52_prs INTERFACE tos_core nrf5-sdk-drivers)

add_library(nrf52_gpio include/arch/gpio.hpp src/gpio.cpp)
target_link_libraries(nrf52_gpio PUBLIC tos_util tos_gpio tos_core nrf5-sdk-drivers)
tos_install(nrf52_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(nrf52_usart PUBLIC tos_core nrf5-sdk-drivers)
tos_install(nrf52_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_timer include/arch/timer.hpp src/timer.cpp)
target_link_libraries(nrf52_timer PUBLIC tos_core nrf5-sdk-drivers)
tos_install(nrf52_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_twim include/arch/twim.hpp src/twim.cpp)
target_link_libraries(nrf52_twim PUBLIC tos_core nrf5-sdk-drivers)
tos_install(nrf52_twim ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_spim include/arch/spi.hpp src/stub.cpp)
target_link_libraries(nrf52_spim PUBLIC tos_core nrf5-sdk-drivers)
tos_install(nrf52_spim ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_radio include/arch/radio.hpp src/radio.cpp)
target_link_libraries(nrf52_radio PUBLIC tos_core)
tos_install(nrf52_radio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_flash include/arch/flash.hpp src/flash.cpp)
target_link_libraries(nrf52_flash PUBLIC tos_core nrf5-sdk-drivers)
tos_install(nrf52_flash ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_ble include/arch/ble.hpp src/ble.cpp include/arch/ble/softdev.hpp include/arch/ble/advertising.hpp src/ble/advertising.cpp include/arch/ble/events.hpp src/ble/softdev.cpp include/arch/ble/common.hpp include/arch/ble/gap.hpp include/arch/ble/gatt.hpp src/ble/gap.cpp src/ble/gatt.cpp)
target_link_libraries(nrf52_ble PUBLIC tos_core)
tos_install(nrf52_ble ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_cc310 include/arch/cc310/rng.hpp src/stub.cpp include/arch/cc310/sha2.hpp include/arch/cc310/cryptocell.hpp src/cc310/cryptocell.cpp include/arch/cc310/aes.hpp)
target_link_libraries(nrf52_cc310 PUBLIC tos_core nrf_cc310 etl)
tos_install(nrf52_cc310 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_drivers src/stub.cpp include/arch/drivers.hpp)
target_link_libraries(nrf52_drivers PUBLIC
    nrf52_gpio
    nrf52_usart
    nrf52_timer
    nrf52_radio
    nrf52_twim
    nrf52_spim
    nrf52_ble
    nrf52_flash
    nrf52_cc310
)
tos_install(nrf52_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(nrf52_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS nrf52_drivers)