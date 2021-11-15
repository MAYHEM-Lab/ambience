add_library(httpparser INTERFACE)
target_include_directories(httpparser INTERFACE src/src)
target_link_libraries(httpparser INTERFACE tos::libcxx)