add_library(gemmlowp INTERFACE)

target_link_libraries(gemmlowp INTERFACE tos::libcxx)
target_include_directories(gemmlowp INTERFACE src)

