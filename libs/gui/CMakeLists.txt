add_library(
  tos_gui
  src/gui.cpp
  include/tos/gui/elements/box.hpp
  src/elements/box.cpp
  include/tos/gui/elements/label.hpp
  src/elements/label.cpp
  include/tos/gui/elements/rbox.hpp
  src/elements/rbox.cpp
  include/tos/gui/element.hpp
  include/tos/gui/elements/horizontal_ruler.hpp
  src/elements/horizontal_ruler.cpp
  include/tos/gui/decorators.hpp
  include/tos/gui/elements.hpp
  include/tos/gui/draw_context.hpp
  include/tos/gui/theme.hpp
  include/tos/gui/size.hpp
  src/size.cpp
  include/tos/gui/elements/placeholder.hpp
)
target_include_directories(tos_gui PUBLIC include)
target_link_libraries(tos_gui PUBLIC tos_gfx2)
