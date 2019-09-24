add_library(tos_gfx stub.cpp include include/tos/gfx/dimensions.hpp include/tos/gfx/transformation.hpp include/tos/gfx/paint.hpp)
target_link_libraries(tos_gfx PUBLIC frozen)
target_compile_features(tos_gfx PUBLIC cxx_std_17)
tos_install(tos_gfx ${CMAKE_CURRENT_SOURCE_DIR}/include)