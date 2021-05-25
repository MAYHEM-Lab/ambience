include(lidlc)

add_lidlc(caplets_schema caplets.lidl)
target_include_directories(caplets_schema INTERFACE .)

add_lidlc(caplets_timeout_schema timeout.lidl)
target_include_directories(caplets_timeout_schema INTERFACE .)

add_library(tos_caplets caplets.cpp include/tos/caplets/token.hpp)
target_link_libraries(tos_caplets PUBLIC tos_util_core emsha caplets_schema)
target_include_directories(tos_caplets PUBLIC include)