project(tos)

add_library(tos_util
        src/stub.cpp
        include/tos/intrusive_list.hpp
        include/tos/intrusive_ptr.hpp
        include/tos/ring_buf.hpp
        include/tos/expected.hpp
        include/tos/function_ref.hpp
        include/tos/utility.hpp
        include/tos/type_traits.hpp include/tos/tuple.hpp include/tos/algorithm.hpp src/new.cpp include/tos/new.hpp include/tos/memory.hpp include/tos/debug.hpp src/cxxrt.cpp)
target_include_directories(tos_util PUBLIC "include")
target_compile_options(tos_util PUBLIC "-std=c++14")