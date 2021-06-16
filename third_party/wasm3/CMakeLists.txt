add_subdirectory(src/source)
add_subdirectory(src/platforms/cpp/wasm3_cpp)

if (TARGET picolibc)
    target_link_libraries(m3 PUBLIC picolibc)
endif()

target_link_libraries(m3 PUBLIC wasm3_cpp)