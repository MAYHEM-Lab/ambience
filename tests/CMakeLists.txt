project(tos)

add_executable(util_tests util_test.cpp)
target_link_libraries(util_tests PUBLIC tests_main tos_util)

add_executable(intrusive_tests intrusive_list_test.cpp)
target_link_libraries(intrusive_tests PUBLIC standalone_tests_main tos_util_core)

add_executable(xbee_tests xbee_parse_test.cpp)
target_link_libraries(xbee_tests PUBLIC standalone_tests_main tos_xbee)

add_executable(ft_tests ft_tests.cpp future_tests.cpp recursive_mutex_test.cpp)
target_link_libraries(ft_tests PUBLIC tests_main)

add_executable(expected_tests expected_test.cpp)
target_link_libraries(expected_tests PUBLIC standalone_tests_main tos_util_core)

add_executable(alarm_tests alarm_tests.cpp)
target_link_libraries(alarm_tests PUBLIC tests_main arch_drivers)

add_executable(rn2903_test rn2903_test.cpp)
target_link_libraries(rn2903_test PUBLIC standalone_tests_main tos::rn2903)

add_executable(function_ref_tests function_ref_tests.cpp)
target_link_libraries(function_ref_tests PUBLIC standalone_tests_main tos_util_core)

add_executable(print_tests print_tests.cpp)
target_link_libraries(print_tests PUBLIC standalone_tests_main tos_print)

add_executable(streams_test streams_test.cpp)
target_link_libraries(streams_test PUBLIC standalone_tests_main tos_print tos_util_core)

add_test(NAME xbee_tests COMMAND xbee_tests)
add_test(NAME intrusive_list_tests COMMAND intrusive_tests)
add_test(NAME utilities_tests COMMAND util_tests)
add_test(NAME ft_tests COMMAND ft_tests)
add_test(NAME expected_tests COMMAND expected_tests)
add_test(NAME alarm_tests COMMAND alarm_tests)
add_test(NAME rn2903_test COMMAND rn2903_test)
add_test(NAME function_ref_tests COMMAND function_ref_tests)
add_test(NAME print_tests COMMAND print_tests)
add_test(NAME streams_test COMMAND streams_test)

add_subdirectory(io)
add_subdirectory(context)