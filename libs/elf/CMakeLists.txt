add_library(tos_elf elf.cpp include/tos/elf.hpp include/tos/elf/common.hpp include/tos/elf/header.hpp include/tos/elf/machine.hpp include/tos/elf/type.hpp include/tos/elf/program_header.hpp include/tos/elf/magic.hpp)
target_include_directories(tos_elf PUBLIC include)
target_link_libraries(tos_elf PUBLIC tos_util_core)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()