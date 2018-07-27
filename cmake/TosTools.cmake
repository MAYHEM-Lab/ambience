function(print_size target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:${target}>
            COMMENT "Print size"
    )
endfunction()