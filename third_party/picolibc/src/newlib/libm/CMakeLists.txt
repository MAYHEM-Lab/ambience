target_include_directories(picolibc PUBLIC common)

target_sources(picolibc PUBLIC common/sf_nan.c)
target_sources(picolibc PUBLIC common/s_nan.c)

add_subdirectory(math)
