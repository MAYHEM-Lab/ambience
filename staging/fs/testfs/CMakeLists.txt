add_library(tos_testfs testfs.cpp)
target_link_libraries(tos_testfs PUBLIC tos_vfs lfs)
