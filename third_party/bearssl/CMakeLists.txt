add_subdirectory(src)
target_sources(bearssl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stub.cpp)
target_include_directories(bearssl PUBLIC include)

add_library(ecdhe_bench ecdhe_bench.cpp)
target_link_libraries(ecdhe_bench PUBLIC BearSSL::BearSSL ubench)
add_benchmark(ecdhe_bench)