add_library(tflite-micro "")

target_link_libraries(tflite-micro PUBLIC tos::libcxx)
target_include_directories(tflite-micro PUBLIC src)

target_sources(
    tflite-micro PRIVATE
    src/tensorflow/lite/micro/all_ops_resolver.cc
)

target_link_libraries(tflite-micro PUBLIC flatbuffers)