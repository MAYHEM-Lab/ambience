function(default_include_dirs)
    string(REPLACE " " ";" CMAKE_CXX_FLAGS_LIST ${CMAKE_CXX_FLAGS})
    set(ARGS_SEPARATOR " ")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS_LIST} -xc++ -E -v -
        COMMAND_ECHO STDERR
        RESULT_VARIABLE COMPILER_RES
        OUTPUT_VARIABLE COMPILER_OUT
        ERROR_VARIABLE COMPILER_ERR)

    string(REGEX MATCH "#include \\<\\.\\.\\.\\> search starts here:\n(.*)End of search list." DIR_MATCHES ${COMPILER_ERR})
    string(REPLACE "\n" ";" DIR_LIST ${DIR_MATCHES})
    list(POP_BACK DIR_LIST)
    list(POP_FRONT DIR_LIST)

    foreach (PATH ${DIR_LIST})
        string(STRIP ${PATH} PATH)
        list(APPEND FINAL_DIR_LIST ${PATH})
    endforeach()

    set(DEFAULT_INCLUDE_DIRS ${FINAL_DIR_LIST} PARENT_SCOPE)
endfunction()