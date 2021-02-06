add_library(vga_text vga_text.cpp)
target_include_directories(vga_text PUBLIC include)
target_link_libraries(vga_text PUBLIC tos_drivers)