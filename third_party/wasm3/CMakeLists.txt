add_subdirectory(src/source)

if (TARGET picolibc)
    target_link_libraries(m3 PUBLIC picolibc)
endif()