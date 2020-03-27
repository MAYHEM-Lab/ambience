add_executable(io_tests serial_multiplexer_test.cpp)
target_link_libraries(io_tests PUBLIC tests_main tos_print)

add_test(NAME io_tests COMMAND io_tests)