add_library(jsmn "" jsmn_lidl.hpp)

target_sources(jsmn PRIVATE jsmn.c)

target_include_directories(jsmn PUBLIC src)
target_include_directories(jsmn PUBLIC .)

add_library(jsmn_lidl INTERFACE)
target_sources(jsmn_lidl INTERFACE jsmn_lidl.hpp)
target_link_libraries(jsmn_lidl INTERFACE jsmn lidl_rt fast_float)