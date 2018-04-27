cmake_minimum_required(VERSION 3.10)
project(tvm)

add_library(tvm_core include/tvm/meta.hpp include/tvm/tvm_types.hpp include/tvm/vm_state.hpp
        include/tvm/traits.hpp src/tests.cpp include/tvm/instr_traits.hpp
        include/tvm/decoding.hpp include/tvm/execution.hpp include/tvm/isa.hpp)
target_include_directories(tvm_core PUBLIC "include")
target_compile_features(tvm_core PUBLIC cxx_std_14)

add_library(tvm_disas include/tvm/dis/disassemble.hpp)
target_link_libraries(tvm_disas PUBLIC tvm_core)
target_compile_features(tvm_disas PUBLIC cxx_std_17)

add_library(tvm_as include/tvm/as/scanner.hpp include/tvm/as/tokens.hpp src/scanner.cpp include/tvm/as/token.hpp)
target_link_libraries(tvm_as PUBLIC tvm_core)
target_compile_features(tvm_as PUBLIC cxx_std_17)

add_executable(ti src/main.cpp include/tvm/instructions.hpp)
target_link_libraries(ti PUBLIC tvm_core)
target_compile_features(ti PUBLIC cxx_std_14)

add_executable(tdis src/dismain.cpp)
target_link_libraries(tdis PUBLIC tvm_core tvm_disas)
target_compile_features(tdis PUBLIC cxx_std_17)

add_executable(tas src/tasmain.cpp)
target_link_libraries(tas PUBLIC tvm_core tvm_as)
target_compile_features(tas PUBLIC cxx_std_17)