add_library(kthreads include/kt/kt.h kt.cpp)
target_include_directories(kthreads PUBLIC include)
target_link_libraries(kthreads PUBLIC tos_ft)