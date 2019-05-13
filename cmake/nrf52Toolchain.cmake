include(ArmToolchain)

set(TOS_NRF52_FLAGS "-mcpu=${TOS_CPU_ARCH} -DNRF52 -DFLOAT_ABI_HARD -DCONFIG_GPIO_AS_PINRESET -D${NRFSDK_DEF} -DNRF52_PAN_74 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float -fno-builtin -fshort-enums -g3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOS_NRF52_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_NRF52_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Xlinker -Map=output.map -fno-threadsafe-statics ${TOS_NRF52_FLAGS}")

SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")

set(SDK_ROOT /opt/x-tools/nRF5_SDK_15.2.0_9412b96)

include_directories(${SDK_ROOT}/components/toolchain/cmsis/include/)
include_directories(${SDK_ROOT}/integration/nrfx)
include_directories(${SDK_ROOT}/modules/nrfx)
include_directories(${SDK_ROOT}/modules/nrfx/mdk/)
include_directories(${SDK_ROOT}/modules/nrfx/hal)
include_directories(${SDK_ROOT}/modules/nrfx/drivers/include)
include_directories(${SDK_ROOT}/components/libraries/delay)
include_directories(${SDK_ROOT}/components/libraries/log)
include_directories(${SDK_ROOT}/components/libraries/log/src)
include_directories(${SDK_ROOT}/components/libraries/experimental_section_vars)
include_directories(${SDK_ROOT}/components/libraries/util)
include_directories(${SDK_ROOT}/components/drivers_nrf/nrf_soc_nosd)