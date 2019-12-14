set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

include(FindToolchain)
find_gnu_toolchain(TRIPLE arm-none-eabi SET)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
