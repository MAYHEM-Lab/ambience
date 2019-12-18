find_program(FLATBUFFERS_FLATC_EXECUTABLE 
    NAMES flatc
    HINTS C:/x-tools/flatbuffers/bin /opt/x-tools/flatbuffers/bin)
    
message(STATUS "Flatc: ${FLATBUFFERS_FLATC_EXECUTABLE}")

if(NOT ${FLATBUFFERS_FLATC_EXECUTABLE} MATCHES "NOTFOUND")
    function(FLATBUFFERS_GENERATE_C_HEADERS Name Subdir)
        set(FLATC_OUTPUTS)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${Subdir})
        foreach(FILE ${ARGN})
            get_filename_component(FLATC_OUTPUT ${FILE} NAME_WE)
            set(FLATC_OUTPUT
                    "${CMAKE_CURRENT_BINARY_DIR}/${Subdir}/${FLATC_OUTPUT}_generated.h")
            list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})
            SET_SOURCE_FILES_PROPERTIES(${FLATC_OUTPUT} PROPERTIES GENERATED 1)

            add_custom_command(OUTPUT ${FLATC_OUTPUT}
                    COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE}
                    ARGS --scoped-enums -c -o "${CMAKE_CURRENT_BINARY_DIR}/${Subdir}/" ${FILE}
                    DEPENDS ${FILE}
                    COMMENT "Building C++ header for ${FILE}"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endforeach()

        add_custom_target(${Name}_IMPL DEPENDS ${FLATC_OUTPUTS})

        add_library(${Name} INTERFACE)
        target_include_directories(${Name} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
        target_link_libraries(${Name} INTERFACE flatbuffers)
        add_dependencies(${Name} ${Name}_IMPL)
    endfunction()
else()
    function(FLATBUFFERS_GENERATE_C_HEADERS Name Subdir)
        message(ERROR "flatc executable not found!")
    endfunction()
endif()
