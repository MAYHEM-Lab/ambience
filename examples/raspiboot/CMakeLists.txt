add_executable(raspiboot main.cpp)
target_link_libraries(raspiboot PUBLIC tos_core arch_drivers tos_print 
    tos_gfx_fonts tos_dynamic_log bcm2837_clock)