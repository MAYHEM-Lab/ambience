# Compile demo

add_executable(pthread_demo main.cpp)
target_link_libraries(pthread_demo PUBLIC tos_core arch_drivers tos_print tos_dynamic_log tos_pthread)

# Compile unit tests

add_executable(pthread_tests pthread_tests.cpp)
target_link_libraries(pthread_tests PUBLIC tos_core arch_drivers tos_print tos_dynamic_log tos_pthread)

add_executable(pthread_cond_test pthread_cond_test.cpp)
target_link_libraries(pthread_cond_test PUBLIC tos_core arch_drivers tos_print tos_dynamic_log tos_pthread)

add_executable(pthread_rwlock_test pthread_rwlock_test.cpp)
target_link_libraries(pthread_rwlock_test PUBLIC tos_core arch_drivers tos_print tos_dynamic_log tos_pthread)
