add_library(boost_hana INTERFACE)
target_include_directories(boost_hana INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_compile_features(boost_hana INTERFACE cxx_std_14)
