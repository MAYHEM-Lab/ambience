find_file(FLOAT_HEADER "float.h"
        PATHS ${DEFAULT_INCLUDE_DIRS}
        NO_DEFAULT_PATH)
message(STATUS "${FLOAT_HEADER}")
get_filename_component(FLOAT_DIR ${FLOAT_HEADER} DIRECTORY)
message(STATUS "${FLOAT_DIR}")

message(STATUS "${DEFAULT_INCLUDE_DIRS}")

find_file(LIMITS_HEADER "limits.h"
        PATHS ${DEFAULT_INCLUDE_DIRS}
        NO_DEFAULT_PATH)
message(STATUS "${LIMITS_HEADER}")
get_filename_component(LIMITS_DIR ${LIMITS_HEADER} DIRECTORY)
message(STATUS "${LIMITS_DIR}")

add_library(picolibc "" include/picolibc.h)
target_include_directories(picolibc PUBLIC include)
target_include_directories(picolibc PUBLIC tinystdio)
target_compile_options(picolibc PUBLIC "-I${FLOAT_DIR}")
target_compile_options(picolibc PUBLIC "-I${LIMITS_DIR}")

target_compile_options(picolibc PUBLIC -nostdlib -nostdinc)
target_compile_options(picolibc PRIVATE -fno-builtin)

add_subdirectory(string)

add_subdirectory(machine)