add_library(
  aekernellib stub.cpp
              include/tos/ae/kernel/runners/group_runner.hpp
              include/tos/ae/kernel/group.hpp)
target_link_libraries(aekernellib PUBLIC aecommon tos_arch_core tos_ft)
target_include_directories(aekernellib PUBLIC include)

add_library(aekernel "")
target_sources(
  aekernel
  PRIVATE
        include/tos/ae/transport/caplets/adapter.hpp
        src/transport/xbee/xbee_host.cpp
        src/transport/xbee/xbee_transport.cpp
        src/transport/http/host.cpp
)

target_link_libraries(aekernel PRIVATE calc_schema)

target_link_libraries(
  aekernel
  PUBLIC tos_util_core
         tos_core
         tos_dynamic_log
         aekernellib
         lidl_log_sink
         alarm_schema
         hana
         tos_xbee
         import_export_schema
         httpparser
         jsmn
         fast_float
         lwipcore
)

if (NOT ${TOS_PLATFORM} MATCHES "hosted")
    target_sources(
            aekernel
            PRIVATE include/tos/ae/transport/lwip/host.hpp
            src/transport/lwip/host.cpp
            include/tos/ae/transport/lwip/udp.hpp
            src/transport/lwip/udp.cpp
            include/tos/ae/transport/caplets/adapter.hpp
    )
    target_link_libraries(
            aekernel
            PUBLIC
            lwipcore
    )
endif()

add_library(ae_xbee_transports INTERFACE)
target_link_libraries(ae_xbee_transports INTERFACE aekernel)
target_sources(ae_xbee_transports INTERFACE src/xbee_exporter.cpp)
target_compile_definitions(ae_xbee_transports INTERFACE TOS_AE_HAVE_XBEE)

add_library(aekernel_node INTERFACE)
target_link_libraries(aekernel_node INTERFACE aekernel)
target_sources(
    aekernel_node
    INTERFACE
        src/node.cpp
)

add_subdirectory(platform)

if (${TOS_ARCH_HAVE_ADDRESS_SPACE})
    add_subdirectory(user_space)
endif()