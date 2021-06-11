project(tos_core CXX ASM)

target_sources(stm32_hal_impl PRIVATE common.cpp vectors.cpp)
target_compile_definitions(stm32_hal_impl PRIVATE -DSTM32_NAME=${LOWER_CPU_NAME})

add_library(stm32_hal_weak_irq weak_irq.cpp)
target_link_libraries(stm32_hal_weak_irq PRIVATE Boost::preprocessor)
target_compile_definitions(stm32_hal_weak_irq PRIVATE -DSTM32_NAME=${LOWER_CPU_NAME})
target_compile_features(stm32_hal_weak_irq PRIVATE cxx_std_20)

target_link_libraries(stm32_hal_impl PUBLIC stm32_hal_weak_irq)