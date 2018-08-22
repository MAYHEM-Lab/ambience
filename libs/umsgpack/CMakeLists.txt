
add_library(umsgpack src/umsgpack.c include/umsgpack.h include/umsgpack.hpp)
target_include_directories(umsgpack PUBLIC include)
target_compile_definitions(umsgpack PRIVATE UMSGPACK_FUNC_INT32 UMSGPACK_LITTLE_ENDIAN)
target_link_libraries(umsgpack PUBLIC tos_util)