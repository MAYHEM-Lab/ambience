add_executable(io_tests serial_multiplexer_test.cpp)
target_link_libraries(io_tests catch_main tos_print)

add_test(io_tests io_tests)