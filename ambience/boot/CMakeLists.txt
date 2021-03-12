include(lidlc)

add_lidlc(aeboot_schema aeboot.yaml)

add_executable(ambience_boot common.cpp)
target_link_libraries(ambience_boot PUBLIC tos_core tos_dynamic_log aeboot_schema)
target_link_libraries(ambience_boot PRIVATE --defsym=Bootloader=0)

if (${TOS_PLATFORM} MATCHES "stm32")
    target_sources(ambience_boot PRIVATE stm32.cpp)
    target_link_libraries(ambience_boot PRIVATE tos_board)
endif()