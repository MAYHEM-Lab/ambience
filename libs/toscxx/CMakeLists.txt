if (TOS_PROVIDE_LIBCXX)
add_library(tos_libcxx
    src/vector.cpp
    include/initializer_list
    include/algorithm
    include/utility
    include/type_traits
    include/__config
    include/version
    include/memory
    include/cstddef
    include/iosfwd
    include/functional
    include/__functional_base
    include/__nullptr)

target_include_directories(tos_libcxx PUBLIC "include")
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_THREADS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_NO_EXCEPTIONS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_NO_RTTI)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_UNICODE_CHARS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_WCHAR)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STDIO)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STDIN _LIBCPP_HAS_NO_STDOUT)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_GLOBAL_FILESYSTEM_NAMESPACE)
target_compile_definitions(tos_libcxx PUBLIC TOS)

target_compile_features(tos_libcxx PUBLIC cxx_std_17)
endif()

add_library(base_libcxx src/compile.cpp)
if (TOS_PROVIDE_LIBCXX)
    target_link_libraries(base_libcxx PUBLIC tos_libcxx)
else()
    target_compile_features(base_libcxx PUBLIC cxx_std_17)
endif()
add_library(tos::libcxx ALIAS base_libcxx)
