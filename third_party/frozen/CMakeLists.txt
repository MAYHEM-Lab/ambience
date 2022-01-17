add_library(frozen INTERFACE)
add_library(frozen::frozen ALIAS frozen)

target_compile_features(frozen INTERFACE cxx_std_20)
target_compile_definitions(frozen INTERFACE FROZEN_NO_EXCEPTIONS)
target_include_directories(frozen INTERFACE src/include)
