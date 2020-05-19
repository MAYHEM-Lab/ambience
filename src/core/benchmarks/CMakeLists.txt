include(TosBench)

add_library(context_bench context_bench.cpp)
target_link_libraries(context_bench PUBLIC tos_core ubench)
add_benchmark(context_bench)

add_library(memory_bench memory_bench.cpp)
target_link_libraries(memory_bench PUBLIC tos_core ubench)
add_benchmark(memory_bench)