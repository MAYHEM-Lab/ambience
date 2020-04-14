add_library(tos_mmio src/mmio.cpp)
target_include_directories(tos_mmio PUBLIC include)
target_compile_features(tos_mmio PUBLIC cxx_std_17)
target_link_libraries(tos_mmio PUBLIC tos_util_core)