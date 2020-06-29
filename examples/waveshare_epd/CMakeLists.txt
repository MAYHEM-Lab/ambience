add_lidlc(painter_schema painter.yaml)
add_lidlc(gfx_schema gfx.yaml)

add_executable(waveshare_epd_demo main.cpp)
target_link_libraries(waveshare_epd_demo PRIVATE
        tos_core
        arch_drivers
        tos_dynamic_log
        tos_device_waveshare29bw
        painter_schema gfx_schema)