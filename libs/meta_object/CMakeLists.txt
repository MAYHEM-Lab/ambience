add_library(tos_meta_object meta_object.cpp)
target_link_libraries(tos_meta_object PUBLIC tos_util_core fmt)
target_include_directories(tos_meta_object PUBLIC include)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()