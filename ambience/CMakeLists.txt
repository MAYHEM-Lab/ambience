add_library(
  aecommon
  src/common.cpp
  include/tos/ae/service_host.hpp
  include/tos/ae/group.hpp
  include/tos/ae/caplets_service_host.hpp
  include/tos/ae/registry.hpp
  include/tos/ae/exporter.hpp
  include/tos/ae/importer.hpp
  include/tos/ae/detail/handle_req.hpp
  src/rings.cpp
)
target_include_directories(aecommon PUBLIC include)
target_link_libraries(aecommon PUBLIC tos_util_core tos_fiber tos_caplets)

add_subdirectory(boot)
add_subdirectory(user_space)
add_subdirectory(kernel_space)
add_subdirectory(services)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
