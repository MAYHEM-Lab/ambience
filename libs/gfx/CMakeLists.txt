add_library(tos_gfx stub.cpp include include/tos/gfx/dimensions.hpp include/tos/gfx/transformation.hpp include/tos/gfx/paint.hpp include/tos/gfx/color.hpp)
target_link_libraries(tos_gfx PUBLIC frozen tos::libcxx tos_util_core tos_debug)
target_compile_features(tos_gfx PUBLIC cxx_std_17)
tos_install(tos_gfx ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(tos_gfx PRIVATE 
    src/truetype.cpp 
    src/stb_truetype.cpp)

add_library(tos_gfx_fonts_opensans_regular src/fonts/opensans_regular.cpp)
target_link_libraries(tos_gfx_fonts_opensans_regular PUBLIC tos_util_core)
target_include_directories(tos_gfx_fonts_opensans_regular PUBLIC include)

add_library(tos_gfx_fonts_ubuntu_regular src/fonts/ubuntu_regular.cpp)
target_link_libraries(tos_gfx_fonts_ubuntu_regular PUBLIC tos_util_core)
target_include_directories(tos_gfx_fonts_ubuntu_regular PUBLIC include)

add_library(tos_gfx_fonts_firacode_regular src/fonts/firacode_regular.cpp)
target_link_libraries(tos_gfx_fonts_firacode_regular PUBLIC tos_util_core)
target_include_directories(tos_gfx_fonts_firacode_regular PUBLIC include)

add_library(tos_gfx_fonts_notosans_regular src/fonts/notosans_regular.cpp)
target_link_libraries(tos_gfx_fonts_notosans_regular PUBLIC tos_util_core)
target_include_directories(tos_gfx_fonts_notosans_regular PUBLIC include)

add_library(tos_gfx_fonts INTERFACE)
target_link_libraries(tos_gfx_fonts INTERFACE
    tos_gfx_fonts_opensans_regular
    tos_gfx_fonts_ubuntu_regular
    tos_gfx_fonts_firacode_regular
    tos_gfx_fonts_notosans_regular)

if (BUILD_TESTS)
    add_executable(truetype_test src/truetype_test.cpp)
    target_link_libraries(truetype_test PRIVATE tos_gfx tests_main tos_gfx_fonts_opensans_regular)
    add_test(NAME truetype_test COMMAND truetype_test)

    add_executable(bitmap_test src/bitmap_test.cpp)
    target_link_libraries(bitmap_test PRIVATE tos_gfx tests_main)
    add_test(NAME bitmap_test COMMAND bitmap_test)
endif()

include(TosBench)
add_library(truetype_bench src/truetype_bench.cpp)
target_link_libraries(truetype_bench PUBLIC ubench tos_gfx tos_gfx_fonts_opensans_regular)
add_benchmark(truetype_bench)
