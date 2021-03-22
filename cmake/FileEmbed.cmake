find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(EMBED_DIR ${CMAKE_CURRENT_LIST_DIR})

function(embed_file TARGET_NAME VARIABLE_NAME FILE_PATH)
    string(MD5 OUTPUT_TARGET ${FILE_PATH})
    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}.h")

    add_custom_command(
        COMMAND Python3::Interpreter "${TOS_PROJECT_ROOT}/tools/dev/bin2array.py" ${FILE_PATH} -O ${OUTPUT_FILE}
        OUTPUT "${OUTPUT_FILE}"
        DEPENDS "${FILE_PATH}"
    )
    add_custom_target("embed_${OUTPUT_TARGET}" DEPENDS "${OUTPUT_FILE}")

    configure_file(${EMBED_DIR}/file_embed.cpp.in "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}.cpp" @ONLY)
    configure_file(${EMBED_DIR}/file_embed.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/${VARIABLE_NAME}.hpp" @ONLY)

    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}.cpp")
    target_sources(${TARGET_NAME} PRIVATE "${OUTPUT_FILE}")

    add_dependencies(${TARGET_NAME} "embed_${OUTPUT_TARGET}")
endfunction()