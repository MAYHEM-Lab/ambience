include(lidlc)

add_lidlc(threadman_schema this_thread.yaml)

add_executable(fault_demo main.cpp unpriv.cpp)
target_link_libraries(fault_demo PRIVATE tos_board tos_core tos_dynamic_log tos_interrupt_trampoline threadman_schema)