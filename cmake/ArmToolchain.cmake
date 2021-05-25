set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

include(FindToolchain)
find_llvm_toolchain(TRIPLE arm-none-eabi SET)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_LINKER ${TOOLCHAIN_LD})
message(STATUS ${CMAKE_LINKER})
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> -o <TARGET> --lto-whole-program-visibility --lto-O3 -z norelro --gc-sections <CMAKE_CXX_LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES>")

set(TOS_BARE_LINKER ON)