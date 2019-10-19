find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
message(STATUS "Flatc: ${FLATBUFFERS_FLATC_EXECUTABLE}")

if(NOT ${FLATBUFFERS_FLATC_EXECUTABLE} MATCHES "NOTFOUND")
    function(FLATBUFFERS_GENERATE_C_HEADERS Name)
        set(FLATC_OUTPUTS)
        foreach(FILE ${ARGN})
            get_filename_component(FLATC_OUTPUT ${FILE} NAME_WE)
            set(FLATC_OUTPUT
                    "${CMAKE_CURRENT_BINARY_DIR}/${FLATC_OUTPUT}_generated.h")
            list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})

            add_custom_command(OUTPUT ${FLATC_OUTPUT}
                    COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE}
                    ARGS --scoped-enums -c -o "${CMAKE_CURRENT_BINARY_DIR}/" ${FILE}
                    DEPENDS ${FILE}
                    COMMENT "Building C++ header for ${FILE}"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endforeach()
        set(${Name}_OUTPUTS ${FLATC_OUTPUTS} PARENT_SCOPE)
    endfunction()
endif()
