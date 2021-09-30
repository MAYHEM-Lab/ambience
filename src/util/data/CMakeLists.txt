add_library(tos_data compile.cpp include/tos/data/detail/heap.hpp include/tos/data/mutable_heap.hpp include/tos/data/detail/vector_storage.hpp)
target_link_libraries(tos_data PUBLIC tos::libcxx tos_util_core)
target_include_directories(tos_data PUBLIC include)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()