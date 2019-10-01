project(tos)

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE include)

add_library(catch_main tests_main.cpp)
target_compile_features(catch_main PUBLIC cxx_std_14)
target_compile_options(catch_main PUBLIC -fexceptions)
target_link_libraries(catch_main PUBLIC tos::ft tos::core doctest)

add_executable(util_tests util_test.cpp)
target_link_libraries(util_tests catch_main tos_util)

add_executable(intrusive_tests intrusive_list_test.cpp)
target_link_libraries(intrusive_tests catch_main tos_util)

add_executable(xbee_tests xbee_parse_test.cpp)
target_link_libraries(xbee_tests catch_main tos_xbee)

add_executable(ft_tests ft_tests.cpp future_tests.cpp)
target_link_libraries(ft_tests catch_main)

add_executable(expected_tests expected_test.cpp)
target_link_libraries(expected_tests catch_main)

add_executable(alarm_tests alarm_tests.cpp)
target_link_libraries(alarm_tests catch_main x86_drivers)

add_executable(rn2903_test rn2903_test.cpp)
target_link_libraries(rn2903_test catch_main tos::rn2903)

add_executable(function_ref_tests function_ref_tests.cpp)
target_link_libraries(function_ref_tests catch_main)

add_executable(print_tests print_tests.cpp)
target_link_libraries(print_tests catch_main tos_print)

add_test(xbee_tests xbee_tests)
add_test(intrusive_list_tests intrusive_tests)
add_test(utilities_tests util_tests)
add_test(ft_tests ft_tests)
add_test(expected_tests expected_tests)
add_test(alarm_tests alarm_tests)
add_test(rn2903_test rn2903_test)
add_test(function_ref_tests function_ref_tests)
add_test(print_tests print_tests)
