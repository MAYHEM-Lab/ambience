include(lidlc)
add_lidlc(mailbox_proto mailbox.yaml)

add_library(raspi3_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(raspi3_usart PUBLIC tos_usart tos_drivers raspi3_messagebox soc_bcm283x)
target_include_directories(raspi3_usart PUBLIC include)

add_library(raspi3_interrupts include/arch/interrupts.hpp src/interrupts.cpp)
target_link_libraries(raspi3_interrupts PUBLIC tos_drivers soc_bcm283x)
target_include_directories(raspi3_interrupts PUBLIC include)
target_link_options(raspi3_interrupts PUBLIC "-Wl,--undefined=exc_handler")

add_library(raspi3_messagebox include/arch/mailbox.hpp src/mailbox.cpp)
target_link_libraries(raspi3_messagebox PUBLIC tos_drivers mailbox_proto soc_bcm283x)
target_include_directories(raspi3_messagebox PUBLIC include)

add_library(raspi3_framebuffer include/arch/framebuffer.hpp src/framebuffer.cpp)
target_link_libraries(raspi3_framebuffer PUBLIC tos_drivers raspi3_messagebox tos_gfx)
target_include_directories(raspi3_framebuffer PUBLIC include)

add_library(raspi3_system_timer include/arch/system_timer.hpp src/system_timer.cpp)
target_link_libraries(raspi3_system_timer PUBLIC tos_drivers raspi3_interrupts tos_util_core)
target_include_directories(raspi3_system_timer PUBLIC include)

add_library(raspi3_generic_timer include/arch/generic_timer.hpp src/generic_timer.cpp)
target_link_libraries(raspi3_generic_timer PUBLIC tos_drivers raspi3_interrupts tos_util_core)
target_include_directories(raspi3_generic_timer PUBLIC include)

add_library(raspi3_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(raspi3_drivers PUBLIC
    raspi3_usart
    raspi3_interrupts
    raspi3_messagebox
    raspi3_framebuffer
    raspi3_system_timer
    raspi3_generic_timer
)

set_target_properties(raspi3_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS raspi3_drivers)
