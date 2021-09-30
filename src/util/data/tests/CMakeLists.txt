add_executable(heap_tests heap_tests.cpp)
target_link_libraries(heap_tests PUBLIC standalone_tests_main tos_data)
add_test(NAME heap_tests COMMAND heap_tests)

add_executable(mut_heap_tests mut_heap_tests.cpp)
target_link_libraries(mut_heap_tests PUBLIC standalone_tests_main tos_data)
add_test(NAME mut_heap_tests COMMAND mut_heap_tests)