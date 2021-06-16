add_library(fmt INTERFACE)
#target_sources(fmt PRIVATE src/src/format.cc)
target_include_directories(fmt INTERFACE src/include)
target_compile_features(fmt INTERFACE cxx_std_20)
target_link_libraries(fmt INTERFACE tos::libcxx)
target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1 FMT_STATIC_THOUSANDS_SEPARATOR='.')

add_library(fmt_compiles fmt_compiles.cpp)
target_link_libraries(fmt_compiles PRIVATE fmt)