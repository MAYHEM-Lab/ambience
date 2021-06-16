add_library(libcperciva_sha256 sha256.c)
target_include_directories(libcperciva_sha256 PUBLIC .)
target_link_libraries(libcperciva_sha256 PUBLIC tos::libc)

add_library(sha256_bench bench.cpp)
target_link_libraries(sha256_bench PUBLIC libcperciva_sha256 ubench)
add_benchmark(sha256_bench)