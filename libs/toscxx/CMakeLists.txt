
add_library(tos_libcxx
        src/stub.cpp
        include/initializer_list
        include/algorithm
        include/utility
        include/type_traits
        include/__config
        include/version
        include/memory
        include/cstddef
        include/__nullptr)

target_include_directories(tos_libcxx PUBLIC "include")
target_compile_options(tos_libcxx PUBLIC -D_LIBCPP_HAS_NO_THREADS)

add_library(base_libcxx src/stub.cpp)
if (TOS_PROVIDE_LIBCXX)
target_link_libraries(base_libcxx PUBLIC tos_libcxx)
endif()
add_library(tos::libcxx ALIAS base_libcxx)
