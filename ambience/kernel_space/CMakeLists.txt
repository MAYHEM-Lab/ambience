add_library(
  aekernellib include/tos/ae/kernel/syscall.hpp include/tos/ae/kernel/user_group.hpp include/tos/ae/kernel/rings.hpp
              include/tos/ae/kernel/address_space.hpp stub.cpp
              include/tos/ae/kernel/runners/preemptive_user_runner.hpp
              include/tos/ae/kernel/runners/group_runner.hpp
              include/tos/ae/kernel/group.hpp
)
target_link_libraries(aekernellib PUBLIC aecommon)
target_include_directories(aekernellib PUBLIC include)

add_executable(aekernel "")
target_sources(
  aekernel
  PRIVATE platform/kernel.hpp
          platform/kernel.cpp
          include/tos/ae/transport/lwip/host.hpp
          src/transport/lwip/host.cpp
          include/tos/ae/transport/lwip/udp.hpp
          src/transport/lwip/udp.cpp
          include/tos/ae/transport/caplets/adapter.hpp
)
target_link_libraries(
  aekernel
  PUBLIC tos_util_core
         tos_core
         tos_dynamic_log
         aekernellib
         calc_schema
         lidl_log_sink
         lwipcore
         alarm_schema
)

add_subdirectory(common)
add_subdirectory(platform)
