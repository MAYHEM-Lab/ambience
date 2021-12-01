add_library(sqlite src/sqlite3.c port.cpp memvfs.cpp)
target_include_directories(sqlite PUBLIC src)
target_link_libraries(sqlite PUBLIC tos::libc)
target_link_libraries(sqlite PRIVATE tos_util_core tos_debug_log)
target_compile_definitions(sqlite PUBLIC 
    SQLITE_THREADSAFE=0 
    SQLITE_OMIT_LOAD_EXTENSION 
    SQLITE_OMIT_DESERIALIZE=1 
    SQLITE_OS_OTHER=1 
    SQLITE_USE_URI=1 
    SQLITE_TEMP_STORE=3
    SQLITE_OMIT_MEMORYDB=1
)