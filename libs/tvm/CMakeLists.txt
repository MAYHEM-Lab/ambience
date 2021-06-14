cmake_minimum_required(VERSION 3.10)
project(tvm)

add_library(tvm_core include/tvm/meta.hpp include/tvm/tvm_types.hpp
        include/tvm/operand_traits.hpp src/tests.cpp include/tvm/instr_traits.hpp
        include/tvm/exec/decoding.hpp include/tvm/exec/execution.hpp include/tvm/exec/isa.hpp include/tvm/vm_traits.hpp
        include/tvm/exec/executor.hpp)
target_include_directories(tvm_core PUBLIC "include")
target_compile_features(tvm_core PUBLIC cxx_std_14)
target_compile_options(tvm_core PUBLIC "-Wno-c++11-narrowing")

if (TOS)
    target_compile_definitions(tvm_core PUBLIC TOS)
    target_link_libraries(tvm_core PUBLIC tos_util)
endif()

if (${TOS_PLATFORM} MATCHES "hosted")
add_library(tvm_disas include/tvm/dis/disassemble.hpp src/stub.cpp)
target_link_libraries(tvm_disas PUBLIC tvm_core)
target_compile_features(tvm_disas PUBLIC cxx_std_17)

add_library(tvm_as include/tvm/as/scanner.hpp include/tvm/as/tokens.hpp src/as/scanner.cpp include/tvm/as/token.hpp include/tvm/as/parser.hpp src/as/parser.cpp include/tvm/as/ast_printer.hpp include/tvm/as/isa_description.hpp include/tvm/as/codegen.hpp src/as/codegen.cpp)
target_link_libraries(tvm_as PUBLIC tvm_core)
target_compile_features(tvm_as PUBLIC cxx_std_17)
target_compile_options(tvm_as PUBLIC "-fexceptions")
endif()

#add_executable(tdis src/dis/dismain.cpp)
#target_link_libraries(tdis PUBLIC tvm_core tvm_disas)
#target_compile_features(tdis PUBLIC cxx_std_17)

add_subdirectory(vms/simple)