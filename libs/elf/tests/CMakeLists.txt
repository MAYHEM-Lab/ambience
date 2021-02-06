add_executable(elf_tests elf_tests.cpp)
target_link_libraries(elf_tests PUBLIC standalone_tests_main tos_elf)
add_test(NAME elf_tests COMMAND elf_tests)