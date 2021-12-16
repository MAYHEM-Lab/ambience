include(lidlc)

add_lidlc(painter_schema painter.lidl)
add_lidlc(gfx_schema gfx.yaml)

add_library(
  tos_gfx2
  gfx2.cpp
  include/tos/gfx2.hpp
  include/tos/gfx2/painter.hpp
  include/tos/gfx2/bit_painter.hpp
  src/bit_painter.cpp
  include/tos/gfx2/utility.hpp
  include/tos/gfx2/color_convert.hpp
  src/color_convert.cpp
  include/tos/gfx2/buffer_painter.hpp
  src/buffer_painter.cpp
  include/tos/gfx2/bitmap.hpp
  src/bitmap.cpp
)

target_link_libraries(tos_gfx2 PUBLIC tos_util_core)
target_link_libraries(tos_gfx2 PUBLIC painter_schema gfx_schema)
target_link_libraries(tos_gfx2 PRIVATE tos_gfx tos::libcxx)
target_include_directories(tos_gfx2 PUBLIC include)
