add_library(x86_64_pc_board INTERFACE)
target_link_libraries(x86_64_pc_board INTERFACE uart_16550 vga_text)
target_include_directories(x86_64_pc_board INTERFACE "..")