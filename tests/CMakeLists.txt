
project(tos)

add_library(catch_main catch_main.cpp)
target_compile_features(catch_main PUBLIC cxx_std_14)
target_compile_options(catch_main PUBLIC -fexceptions)

add_executable(util_tests util_test.cpp)
target_link_libraries(util_tests catch_main tos_util)

add_executable(intrusive_tests intrusive_list_test.cpp)
target_link_libraries(intrusive_tests catch_main tos_util)

add_executable(xbee_tests xbee_parse_test.cpp)
target_link_libraries(xbee_tests catch_main tos_xbee)