add_library(tinyaes src/aes.c)
target_include_directories(tinyaes PUBLIC src/)

if (BUILD_TESTS)
    add_executable(tinyaes_test src/test.c)
    target_link_libraries(tinyaes_test PUBLIC tinyaes)
    add_test(NAME tinyaes_test COMMAND tinyaes_test)
endif()