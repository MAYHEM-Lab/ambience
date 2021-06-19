# Do not build pthreads on hosted platforms
if (${TOS_PLATFORM} MATCHES "hosted")
    return()
endif()

add_library(tos_pthread threading.cpp condition_variable.cpp shared_mutex.cpp)
target_include_directories(tos_pthread PUBLIC include)
target_link_libraries(tos_pthread PRIVATE tos_ft)
