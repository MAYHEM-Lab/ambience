add_library(cwpack src/cwpack.c src/cwpack.h src/cwpack_defines.h src/cwpack.hpp)
target_include_directories(cwpack PUBLIC src)
target_link_libraries(cwpack PUBLIC tos_util_core)
