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

add_library(picolibc "")
target_compile_options(picolibc PUBLIC -I${FLOAT_DIR})
target_compile_options(picolibc PUBLIC -I${LIMITS_DIR})

target_compile_options(picolibc PUBLIC -nostdlib -nostdinc)
target_compile_options(picolibc PRIVATE -fno-builtin -Wno-sign-compare -Wno-int-conversion -Wno-implicit-function-declaration)

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
    target_compile_options(picolibc PUBLIC -Wno-macro-redefined -Wno-keyword-macro -Wno-gnu-include-next)
    target_compile_options(picolibc PRIVATE -Wno-visibility -Wno-language-extension-token)
elseif(${CMAKE_C_COMPILER_ID} MATCHES "GNU")
    target_compile_options(picolibc PRIVATE -Wno-implicit-fallthrough)
endif()

target_compile_definitions(picolibc PRIVATE HAVE_ALIAS_ATTRIBUTE)

add_subdirectory(libc)
add_subdirectory(libm)

