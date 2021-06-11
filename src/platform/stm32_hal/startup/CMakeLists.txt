project(tos_core CXX ASM)

target_sources(stm32_hal_impl PRIVATE common.cpp irq.cpp vectors.cpp)
target_link_libraries(stm32_hal_impl PRIVATE tos_mem)
target_compile_definitions(stm32_hal_impl PRIVATE -DSTM32_NAME=${LOWER_CPU_NAME})