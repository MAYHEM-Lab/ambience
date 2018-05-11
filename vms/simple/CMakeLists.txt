project(tvm)

add_library(simple_vm instructions.hpp vm_state.hpp vm_def.hpp stub.cpp)
target_link_libraries(simple_vm PUBLIC tvm_core)
target_include_directories(simple_vm PUBLIC ".")

if (TOS)
    add_executable(svm tos_exec.cpp)
    target_link_libraries(svm PUBLIC simple_vm)
    target_link_libraries(svm PUBLIC avr_usart tos_print)
endif()

if (NOT TOS)
add_executable(svm exec.cpp)
target_link_libraries(svm PUBLIC simple_vm)
add_executable(svm_as as.cpp)
target_link_libraries(svm_as PUBLIC simple_vm tvm_as)
endif()