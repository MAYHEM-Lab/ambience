include(lidlc)

add_lidlc(painter_schema painter.yaml)
add_lidlc(gfx_schema gfx.yaml)

add_library(tos_gfx2 gfx2.cpp include/tos/gfx2.hpp include/tos/gfx2/painter.hpp include/tos/gfx2/bit_painter.hpp bit_painter.cpp include/tos/gfx2/utility.hpp)
target_link_libraries(tos_gfx2 PUBLIC tos_util_core)
target_link_libraries(tos_gfx2 PUBLIC
        painter_schema
        gfx_schema)
target_link_libraries(tos_gfx2 PRIVATE tos_gfx)
target_include_directories(tos_gfx2 PUBLIC include)