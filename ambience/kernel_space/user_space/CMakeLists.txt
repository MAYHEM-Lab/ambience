add_library(ae_usergroup 
    include/tos/ae/kernel/runners/preemptive_user_runner.hpp
    include/tos/ae/kernel/user_group.hpp  
    src/user_group.cpp
)

target_include_directories(ae_usergroup PUBLIC include)

target_link_libraries(ae_usergroup PUBLIC aekernellib quikshare tos_elf)

if (${TOS_ARCH} MATCHES "x86_64" OR ${TOS_ARCH} MATCHES "aarch64")
    target_sources(ae_usergroup PRIVATE src/preemptive_user_group.cpp)
    target_sources(ae_usergroup PRIVATE src/in_memory_user_group.cpp)
    target_sources(ae_usergroup PRIVATE "src/start_group_${TOS_ARCH}.cpp")
elseif(${TOS_ARCH} MATCHES "arm")
    target_sources(ae_usergroup PRIVATE src/in_memory_user_group.cpp)
    target_sources(ae_usergroup PRIVATE src/start_group_arm.cpp)
endif()