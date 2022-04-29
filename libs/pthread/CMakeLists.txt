# Do not build pthreads on hosted platforms
if (${TOS_PLATFORM} MATCHES "hosted" OR ${TOS_ARCH} MATCHES "i386")
    add_library(tos_pthread INTERFACE)
    return()
endif()

add_library(tos_pthread threading.cpp include/sched.h sched.cpp)
target_include_directories(tos_pthread PUBLIC include)
target_link_libraries(tos_pthread PRIVATE tos_ft)
