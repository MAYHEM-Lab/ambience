if (NOT ENABLE_TESTS)
    return()
endif()

if (NOT ${TOS_PLATFORM} MATCHES "x86")
    return()
endif()

add_library(doctest INTERFACE)
target_compile_definitions(doctest INTERFACE DOCTEST_CONFIG_NO_POSIX_SIGNALS DOCTEST_NO_XML_REPORTER)
target_include_directories(doctest INTERFACE include)

add_library(standalone_tests_main standalone_tests_main.cpp)
target_compile_features(standalone_tests_main PUBLIC cxx_std_17)
target_compile_options(standalone_tests_main PUBLIC -fexceptions)
target_link_libraries(standalone_tests_main PUBLIC doctest)

add_library(tests_main tests_main.cpp)
target_compile_features(tests_main PUBLIC cxx_std_17)
target_compile_options(tests_main PUBLIC -fexceptions)
target_link_libraries(tests_main PUBLIC tos::core doctest arch_drivers tos_dynamic_log)


