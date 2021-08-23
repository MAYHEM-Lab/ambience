add_library(kiss_fftr_16 "")
target_sources(kiss_fftr_16 PRIVATE
    src/kiss_fft.c
    src/tools/kiss_fftr.c
)
target_include_directories(kiss_fftr_16 PUBLIC src)
target_compile_definitions(kiss_fftr_16 PUBLIC FIXED_POINT=16)
target_link_libraries(kiss_fftr_16 PUBLIC tos::libc)