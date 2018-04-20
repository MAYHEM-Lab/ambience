cmake_minimum_required(VERSION 3.10)
project(tvm)

set(CMAKE_CXX_STANDARD 17)

add_library(tvm include/tvm/meta.hpp include/tvm/tvm_types.hpp include/tvm/vm_state.hpp include/tvm/traits.hpp src/tests.cpp include/tvm/instr_traits.hpp include/tvm/decoding.hpp include/tvm/execution.hpp)
target_include_directories(tvm PUBLIC "include")

add_executable(ti src/main.cpp include/tvm/instructions.hpp src/simple.cpp)
target_link_libraries(ti PUBLIC tvm)

add_executable(tdis src/dismain.cpp)
target_link_libraries(tdis PUBLIC tvm)