project(tos)

add_library(
  tos_util_core
  src/stub.cpp
  include/tos/stack_storage.hpp
  include/tos/compiler.hpp
  include/tos/intrusive_list.hpp
  include/tos/intrusive_ptr.hpp
  include/tos/ring_buf.hpp
  include/tos/function_ref.hpp
  include/tos/utility.hpp
  src/cxxrt.cpp
  include/tos/span.hpp
  include/tos/ct_map.hpp
  include/tos/track_ptr.hpp
  include/tos/mem_stream.hpp
  include/tos/streams.hpp
  include/tos/fixed_point.hpp
  include/nonstd/expected.hpp
  include/tos/uuid.hpp
  include/tos/moved_flag.hpp
  include/tos/functional.hpp
  include/tos/meta/function_traits.hpp
  include/tos/string.hpp
  src/base64.cpp
  src/span.cpp
  include/tos/math/saturated.hpp
  src/math/saturated.cpp
  include/tos/gnu_build_id.hpp
  include/tos/math/nearest_power_of_two.hpp
)

target_compile_features(tos_util_core PUBLIC cxx_std_17)
target_link_libraries(tos_util_core PUBLIC tos::libcxx tos_memory)
target_compile_definitions(tos_util_core PUBLIC TOS_DEFAULT_STACK_SIZE=${TOS_DEF_STACK})

add_library(
  tos_util src/stub.cpp src/expected.cpp include/tos/expected.hpp src/new.cpp include/tos/fixed_fifo.hpp
           include/tos/sync_ring_buf.hpp
)

target_link_libraries(tos_util PUBLIC tos_debug tos_util_core tos::ft)

add_library(tos::util ALIAS tos_util)

tos_install(tos_util ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tos_util PUBLIC tos_memory)
tos_install(tos_util_core ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(serial_mux include/tos/io/serial_multiplexer.hpp src/io/serial_multiplexer.cpp)
target_link_libraries(serial_mux PUBLIC tos_util_core tos::ft)

add_library(tos_packets include/tos/io/packet.hpp src/io/packet.cpp)
target_link_libraries(tos_packets PUBLIC tos_util_core tos::ft)

add_library(tos_serial_packets include/tos/io/serial_packets.hpp src/io/serial_packets.cpp)
target_link_libraries(tos_serial_packets PUBLIC tos_util_core tos::ft tos_packets)

add_library(tos_serial_backend include/tos/io/serial_backend.hpp src/serial_backend.cpp)
target_link_libraries(tos_serial_backend PUBLIC tos_util_core tos::ft tos_packets)

add_library(tos_packet_transport include/tos/io/packet_transport.hpp src/io/packet_transport.cpp)
target_link_libraries(tos_packet_transport PUBLIC tos_util_core tos_packets)

add_library(tos_cancellation_token include/tos/cancellation_token.hpp src/cancellation_token.cpp)
target_link_libraries(tos_cancellation_token PUBLIC tos_util_core)

if(BUILD_TESTS)
  add_executable(uuid_test src/uuid_test.cpp)
  target_link_libraries(uuid_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME uuid_test COMMAND uuid_test)

  add_executable(hex_test src/hex_test.cpp)
  target_link_libraries(hex_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME hex_test COMMAND hex_test)

  add_executable(mem_stream_test src/mem_stream_test.cpp)
  target_link_libraries(mem_stream_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME mem_stream_test COMMAND mem_stream_test)

  add_executable(intrusive_ptr_test src/intrusive_ptr_test.cpp)
  target_link_libraries(intrusive_ptr_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME intrusive_ptr_test COMMAND intrusive_ptr_test)

  add_executable(mem_fn_test src/mem_fn_test.cpp)
  target_link_libraries(mem_fn_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME mem_fn_test COMMAND mem_fn_test)

  add_executable(offsetof_test src/offsetof_test.cpp)
  target_link_libraries(offsetof_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME offsetof_test COMMAND offsetof_test)

  add_executable(split_test src/split_test.cpp)
  target_link_libraries(split_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME split_test COMMAND split_test)

  add_executable(base64_test src/base64_test.cpp)
  target_link_libraries(base64_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME base64_test COMMAND base64_test)

  add_executable(saturated_test src/math/saturated_test.cpp)
  target_link_libraries(saturated_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME saturated_test COMMAND saturated_test)

  add_executable(math_test src/math/math_test.cpp)
  target_link_libraries(math_test PUBLIC standalone_tests_main tos_util_core)
  add_test(NAME math_test COMMAND math_test)

  add_executable(serial_backend_test src/serial_backend_test.cpp)
  target_link_libraries(serial_backend_test PUBLIC tests_main tos_serial_backend)
  add_test(NAME serial_backend_test COMMAND serial_backend_test)

  add_executable(cancellation_token_test src/cancellation_token_test.cpp)
  target_link_libraries(cancellation_token_test PUBLIC standalone_tests_main tos_cancellation_token)
  add_test(NAME cancellation_token_test COMMAND cancellation_token_test)
endif()
