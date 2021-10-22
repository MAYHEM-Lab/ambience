add_library(quikshare quik.cpp)
target_link_libraries(quikshare PUBLIC tos_util_core lidl_rt)
target_include_directories(quikshare PUBLIC include)