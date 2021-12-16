add_library(fmt "")
target_sources(fmt PRIVATE src/src/format.cc)
target_include_directories(fmt PUBLIC src/include)
target_compile_features(fmt PUBLIC cxx_std_20)
target_link_libraries(fmt PUBLIC tos::libcxx)
target_compile_definitions(fmt PUBLIC FMT_STATIC_THOUSANDS_SEPARATOR='.')

add_library(fmt_compiles fmt_compiles.cpp)
target_link_libraries(fmt_compiles PRIVATE fmt)