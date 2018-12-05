
add_library(json include/nlohmann/json.hpp stub.cpp)
target_include_directories(json PUBLIC "include")
target_link_libraries(json PUBLIC tos::libcxx)

add_library(nlohmann::json ALIAS json)