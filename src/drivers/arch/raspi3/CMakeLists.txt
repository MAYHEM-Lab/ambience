add_library(raspi3_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(raspi3_usart PUBLIC tos_usart tos::core)
target_include_directories(raspi3_usart PUBLIC include)

add_library(raspi3_interrupts include/arch/interrupts.hpp src/interrupts.cpp)
target_link_libraries(raspi3_interrupts PUBLIC tos::core)
target_include_directories(raspi3_interrupts PUBLIC include)

add_library(raspi3_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(raspi3_drivers PUBLIC
    raspi3_usart
    raspi3_interrupts
)

set_target_properties(raspi3_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS raspi3_drivers)