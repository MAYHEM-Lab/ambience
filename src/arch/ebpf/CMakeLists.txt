add_library(arch_ebpf wasm.cpp)
target_compile_features(arch_ebpf PUBLIC cxx_std_20)