add_library(aarch64_core core.cpp)
target_include_directories(aarch64_core PUBLIC include)
target_compile_features(aarch64_core PUBLIC cxx_std_17)
target_link_libraries(aarch64_core PUBLIC cmsis::core)

add_library(aarch64_impl impl.cpp boot.s)
target_include_directories(aarch64_impl PUBLIC include)
target_link_libraries(aarch64_impl PUBLIC aarch64_core tos_ft)