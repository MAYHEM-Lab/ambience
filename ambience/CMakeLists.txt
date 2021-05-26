add_library(
  aecommon
  src/common.cpp
  include/tos/ae/entry.hpp
  include/tos/ae/service_host.hpp
  include/tos/ae/group.hpp
  include/tos/ae/transport/downcall.hpp
  include/tos/ae/transport/upcall.hpp
  include/tos/ae/caplets_service_host.hpp
  include/tos/ae/registry.hpp
)
target_include_directories(aecommon PUBLIC include)
target_link_libraries(aecommon PUBLIC tos_util_core tos_caplets)

add_subdirectory(boot)
add_subdirectory(services)
add_subdirectory(user_space)
add_subdirectory(kernel_space)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
