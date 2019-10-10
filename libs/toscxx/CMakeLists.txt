if (TOS_PROVIDE_LIBCXX)
add_library(tos_libcxx
    src/vector.cpp
    src/string.cpp
    src/memory.cpp
    src/include/atomic_support.h
    src/any.cpp

    include/algorithm
    include/array
    include/bit
    include/__bit_reference
    include/cassert
    include/cctype
    include/cerrno
    include/chrono
    include/ciso646
    include/climits
    include/__config
    include/cstdarg
    include/cstddef
    include/cstdint
    include/cstdio
    include/cstdlib
    include/cstring
    include/ctime
    include/cwchar
    include/cwctype
    include/__debug
    include/exception
    include/forward_list
    include/functional
    include/__functional_base
    include/__hash_table
    include/initializer_list
    include/iosfwd
    include/iterator
    include/limits
    include/map
    include/memory
    include/new
    include/__node_handle
    include/__nullptr
    include/numeric
    include/optional
    include/ratio
    include/span
    include/__split_buffer
    include/stdexcept
    include/string
    include/__string
    include/string_view
    include/__tree
    include/tuple
    include/__tuple
    include/typeinfo
    include/type_traits
    include/__undef_macros
    include/unordered_map
    include/utility
    include/valarray
    include/variant
    include/vector
    include/version
    include/wchar.h
    include/wctype.h
    include/cmath
    include/any
)

target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_THREADS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_NO_EXCEPTIONS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_NO_RTTI)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_LONG_LONG)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_INT128)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_UNICODE_CHARS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STOF)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STDIO)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STDOUT)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_STDIN)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_GLOBAL_FILESYSTEM_NAMESPACE)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_ATOMIC_HEADER)
target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_ABI_VERSION=2)

if(${TOS_CORE} MATCHES "atmega")
    target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_FLOATING_POINT)
    target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_WCHAR)
endif()
if(${TOS_CORE} MATCHES "esp8266")
    target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_FLOATING_POINT)
    target_compile_definitions(tos_libcxx PUBLIC _LIBCPP_HAS_NO_WCHAR)
endif()

target_compile_definitions(tos_libcxx PUBLIC TOS)
target_compile_definitions(tos_libcxx PUBLIC TOS_ARCH_${TOS_ARCH_NAME})

target_compile_features(tos_libcxx PUBLIC cxx_std_17)
tos_install(tos_libcxx ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

add_library(base_libcxx src/compile.cpp)
if (TOS_PROVIDE_LIBCXX)
    target_link_libraries(base_libcxx PUBLIC tos_libcxx)
else()
    target_compile_features(base_libcxx PUBLIC cxx_std_17)
endif()
add_library(tos::libcxx ALIAS base_libcxx)

tos_install(base_libcxx)
