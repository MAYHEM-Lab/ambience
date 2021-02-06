project(ti_core CXX C ASM)

add_library(cc32xx_core "")
target_sources(cc32xx_core PRIVATE core.cpp)
target_link_libraries(cc32xx_core PUBLIC ti_sdk tos_util_core cmsis::core ti_driverlib ti_drivers tos_arch_core)
target_include_directories(cc32xx_core PUBLIC include)
target_link_libraries(cc32xx_core PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cc32xx_core PUBLIC "-Tcc3220sf.ld")
target_link_libraries(cc32xx_core PUBLIC "-specs=nano.specs -specs=nosys.specs")
target_link_libraries(cc32xx_core PUBLIC "-Wl,--start-group -lgcc -lm -lc -lg -lstdc++ -lsupc++ -Wl,--end-group")
target_link_libraries(cc32xx_core PUBLIC "-Wl,--undefined=main")

target_compile_features(cc32xx_core PUBLIC cxx_std_17)

add_library(cc32xx_impl "")
target_sources(cc32xx_impl PRIVATE impl.cpp)
target_link_libraries(cc32xx_impl PUBLIC tos_interrupt tos_jobs cc32xx_core)
target_include_directories(cc32xx_impl PUBLIC include)

set_target_properties(cc32xx_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cc3220sf.ld)