project(capabilities)
find_package(FlatBuffers REQUIRED)

FLATBUFFERS_GENERATE_C_HEADERS(caps_flat_headers caps flats/capability.fbs flats/sha2.fbs)

add_library(caps
        src/common.cpp
        src/caps.cpp
        src/emsha_signer.cpp
        include/caps/common.hpp
        include/caps/caps.hpp
        include/caps/emsha_signer.hpp
        include/caps/raw_serdes.hpp
        include/caps/request.hpp)
target_include_directories(caps PUBLIC include)
target_link_libraries(caps PUBLIC tos_util emsha cwpack caps_flat_headers)

if (BUILD_TESTS)
    add_executable(caps_test src/caps_test.cpp)
    target_link_libraries(caps_test PUBLIC doctest tests_main caps)
    add_test(NAME caps_test COMMAND caps_test)
endif()