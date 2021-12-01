add_executable(sqlite_demo main.cpp)
target_link_libraries(sqlite_demo PRIVATE 
    sqlite 
    tos_core 
    tos_board 
    tos_dynamic_log
    fmt
)
add_iso(sqlite-demo-iso sqlite_demo)