project(tos)

add_library(tos_util_core
    src/stub.cpp

    include/tos/stack_storage.hpp
    include/tos/compiler.hpp
    include/tos/intrusive_list.hpp
    include/tos/intrusive_ptr.hpp
    include/tos/ring_buf.hpp
    include/tos/function_ref.hpp
    include/tos/utility.hpp
    src/new.cpp
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
)

target_compile_features(tos_util_core PUBLIC cxx_std_14)
target_link_libraries(tos_util_core PUBLIC tos::libcxx)

add_library(tos_util
        src/stub.cpp
        src/expected.cpp
        include/tos/expected.hpp

        include/tos/fixed_fifo.hpp
        include/tos/sync_ring_buf.hpp
        include/tos/io/serial_multiplexer.hpp
        )

target_link_libraries(tos_util PUBLIC tos_util_core tos::ft)

add_library(tos::util ALIAS tos_util)

tos_install(tos_util ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tos_util PUBLIC "-Wl,--whole-archive $<TARGET_FILE:tos_util_core> -Wl,--no-whole-archive")
target_link_libraries(tos_util PUBLIC tos_memory)
tos_install(tos_util_core ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(serial_mux
        include/tos/io/serial_multiplexer.hpp
        src/io/serial_multiplexer.cpp
        )
target_link_libraries(serial_mux PUBLIC tos_util_core tos::ft)

if (BUILD_TESTS)
    add_executable(uuid_test src/uuid_test.cpp)
    target_link_libraries(uuid_test PUBLIC tests_main tos_util_core)
    add_test(NAME uuid_test COMMAND uuid_test)

    add_executable(hex_test src/hex_test.cpp)
    target_link_libraries(hex_test PUBLIC tests_main tos_util_core)
    add_test(NAME hex_test COMMAND hex_test)

	add_executable(mem_stream_test src/mem_stream_test.cpp)
	target_link_libraries(mem_stream_test PUBLIC tests_main tos_util_core)
    add_test(NAME mem_stream_test COMMAND mem_stream_test)
endif()
