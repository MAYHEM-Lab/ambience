include(TosBench)

add_library(memory_bench memory_bench.cpp)
target_link_libraries(memory_bench PUBLIC tos_core ubench)
add_benchmark(memory_bench)