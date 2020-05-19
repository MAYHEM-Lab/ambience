add_library(ubench include/tos/ubench/bench.hpp ubench.cpp include/tos/ubench/state.hpp)
target_link_libraries(ubench PUBLIC tos_core tos_ft)
target_include_directories(ubench PUBLIC include)
target_compile_features(ubench PUBLIC cxx_std_17)

include(TosBench)

add_library(empty_bench empty_bench.cpp)
target_link_libraries(empty_bench PUBLIC ubench)
add_benchmark(empty_bench)