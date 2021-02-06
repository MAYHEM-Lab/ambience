project(tos C CXX ASM)

add_library(raspi_core core.cpp cxxrt.cpp include/tos/platform.hpp)
target_include_directories(raspi_core PUBLIC include)
target_compile_features(raspi_core PUBLIC cxx_std_17)
target_link_libraries(raspi_core PUBLIC cmsis::core tos_arch_core)
target_compile_definitions(raspi_core PUBLIC __PROGRAM_START=_start)

add_library(raspi_impl impl.cpp boot.S exceptions.S newlib_stubs.cpp mmu_init.cpp)
target_include_directories(raspi_impl PUBLIC include)
target_link_libraries(raspi_impl PUBLIC raspi_core tos_jobs raspi3_drivers)
target_link_libraries(raspi_impl PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
#target_link_libraries(raspi_impl PUBLIC "-Wl,--start-group -lgcc -lm -lc -lg -lstdc++ -lsupc++ -Wl,--end-group -Wl,--undefined=_read")
set_target_properties(raspi_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)
