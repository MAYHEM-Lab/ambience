function(default_include_dirs)
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        return()
    endif()

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
        if (${PATH} MATCHES "c\\+\\+")
            # If a default include dir includes c++ in it, it's likely the standard C++ library headers and they are often
            # just consist of #include_next <...>, which we do not want at all so filter them out.
            continue()
        endif()
        list(APPEND FINAL_DIR_LIST ${PATH})
    endforeach()

    set(DEFAULT_INCLUDE_DIRS ${FINAL_DIR_LIST} PARENT_SCOPE)
endfunction()