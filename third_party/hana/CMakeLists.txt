#add_subdirectory(src)
add_library(hana INTERFACE)
target_include_directories(hana SYSTEM INTERFACE "src/include")
target_compile_features(hana INTERFACE cxx_std_14)
