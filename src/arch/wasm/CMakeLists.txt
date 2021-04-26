add_library(arch_wasm wasm.cpp)
target_compile_features(arch_wasm PUBLIC cxx_std_20)