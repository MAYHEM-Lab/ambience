add_library(libcperciva_sha256 sha256.c)
target_include_directories(libcperciva_sha256 PUBLIC .)

if (TARGET picolibc)
    target_link_libraries(libcperciva_sha256 PUBLIC picolibc)
endif()

add_library(sha256_bench bench.cpp)
target_link_libraries(sha256_bench PUBLIC libcperciva_sha256 ubench)
add_benchmark(sha256_bench)