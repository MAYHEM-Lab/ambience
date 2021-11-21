if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscall/${TOS_ARCH}")
  message(STATUS "ae user space does not exist on ${TOS_ARCH}")
  return()
endif()

add_library(ae_user_space
  user_space.cpp
  include/tos/ae/user_space.hpp
  entry.cpp
  syscall/include/tos/ae/detail/syscall.hpp
  heap.cpp
)
target_link_libraries(
  ae_user_space
  PUBLIC tos_coro
         tos_util_core
         aecommon
         tos_arch_core
         log_schema
         group_schema
         tos_print
         lidl_log_sink
         cxxabi_static
         tos_fiber_single_thread
)
target_link_libraries(ae_user_space PRIVATE tos_mem)
target_compile_definitions(ae_user_space PUBLIC __ambience__)

target_include_directories(ae_user_space PUBLIC include)

add_subdirectory(syscall)

if(NOT ${TOS_ARCH} MATCHES "wasm")
  target_link_libraries(ae_user_space PUBLIC "-L ${CMAKE_CURRENT_SOURCE_DIR}/ld")
else()
  target_link_libraries(ae_user_space PUBLIC "-e _user_code")
endif()
