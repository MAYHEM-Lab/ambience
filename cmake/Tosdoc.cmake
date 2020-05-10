if (NOT BUILD_DOCS)
    function(add_tosdoc Name)
    endfunction()
    return()
endif()

find_program(ASCIIDOCTOR_EXE NAMES asciidoctor)

if (NOT ASCIIDOCTOR_EXE)
    message(STATUS "Asciidoctor not found, documents wont be generated")
    function(add_tosdoc Name)
    endfunction()
    return()
endif()

add_custom_target(tosdoc ALL)

set(DOCS_ROOT ${CMAKE_BINARY_DIR}/tosdoc)
file(MAKE_DIRECTORY ${DOCS_ROOT})

function(add_tosdoc)
    file(RELATIVE_PATH RELATIVE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${DOCS_ROOT}/${RELATIVE})

    string(REPLACE "/" "_" Name ${RELATIVE})

    set(ASCIIDOC_OUTPUTS)
    foreach(FILE ${ARGN})
        get_filename_component(ASCIIDOC_OUTPUT ${FILE} NAME_WE)
        set(ASCIIDOC_OUTPUT ${DOCS_ROOT}/${RELATIVE}/${ASCIIDOC_OUTPUT}.html)

        list(APPEND ASCIIDOC_OUTPUTS ${ASCIIDOC_OUTPUT})
        SET_SOURCE_FILES_PROPERTIES(${ASCIIDOC_OUTPUT} PROPERTIES GENERATED 1)

        add_custom_command(OUTPUT ${ASCIIDOC_OUTPUT}
                COMMAND ${ASCIIDOCTOR_EXE}
                ARGS -b html5 -q -o ${ASCIIDOC_OUTPUT} ${FILE}
                DEPENDS ${FILE}
                COMMENT "Building ${FILE}"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endforeach()

    add_custom_target(tosdocs_${Name} DEPENDS ${ASCIIDOC_OUTPUTS})
    add_dependencies(tosdoc tosdocs_${Name})
endfunction()
