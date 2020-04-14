add_library(ubench include/tos/ubench/bench.hpp ubench.cpp include/tos/ubench/state.hpp)
target_link_libraries(ubench PUBLIC tos_core)
target_include_directories(ubench PUBLIC include)
target_compile_features(ubench PUBLIC cxx_std_17)
