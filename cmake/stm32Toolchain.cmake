include(ArmToolchain)

set(TOS_STM32_FLAGS "-mcpu=cortex-m3 -mabi=aapcs -DSTM32F1 -mthumb -fno-builtin -fshort-enums -g3 -nostartfiles -msoft-float")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOS_STM32_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_STM32_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOS_STM32_FLAGS}")

SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")
