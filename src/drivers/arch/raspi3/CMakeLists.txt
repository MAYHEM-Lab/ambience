add_lidlc(messagebox_proto messagebox.yaml)

add_library(raspi3_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(raspi3_usart PUBLIC tos_usart tos::core raspi3_messagebox)
target_include_directories(raspi3_usart PUBLIC include)

add_library(raspi3_interrupts include/arch/interrupts.hpp src/interrupts.cpp)
target_link_libraries(raspi3_interrupts PUBLIC tos::core)
target_include_directories(raspi3_interrupts PUBLIC include)

add_library(raspi3_messagebox include/arch/messagebox.hpp src/messagebox.cpp)
target_link_libraries(raspi3_messagebox PUBLIC tos::core messagebox_proto)
target_include_directories(raspi3_messagebox PUBLIC include)

add_library(raspi3_framebuffer include/arch/framebuffer.hpp src/framebuffer.cpp)
target_link_libraries(raspi3_framebuffer PUBLIC tos::core raspi3_messagebox tos_gfx)
target_include_directories(raspi3_framebuffer PUBLIC include)

add_library(raspi3_system_timer include/arch/system_timer.hpp src/system_timer.cpp)
target_link_libraries(raspi3_system_timer PUBLIC tos::core raspi3_interrupts tos_util_core)
target_include_directories(raspi3_system_timer PUBLIC include)

add_library(raspi3_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(raspi3_drivers PUBLIC
    raspi3_usart
    raspi3_interrupts
    raspi3_messagebox
    raspi3_framebuffer
    raspi3_system_timer
)

set_target_properties(raspi3_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS raspi3_drivers)