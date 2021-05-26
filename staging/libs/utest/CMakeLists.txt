add_library(utest include/tos/utest/test.hpp utest.cpp)
target_link_libraries(utest PUBLIC tos_core tos_ft)
target_include_directories(utest PUBLIC include)
target_compile_features(utest PUBLIC cxx_std_17)
