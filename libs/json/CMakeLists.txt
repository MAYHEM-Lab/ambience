
add_library(json include/nlohmann/json.hpp stub.cpp)
target_include_directories(json PUBLIC "include")

add_library(nlohmann::json ALIAS json)