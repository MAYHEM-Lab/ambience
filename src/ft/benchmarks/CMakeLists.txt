include(TosBench)

add_library(thread_bench thread_bench.cpp)
target_link_libraries(thread_bench PUBLIC tos_core ubench)
add_benchmark(thread_bench)