set(CROSSCMAKE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "")

function(build_other_target target other_build_dir other_target)
    string(MD5 OUTPUT_TARGET ${other_build_dir}/${other_target})

    add_custom_target(${OUTPUT_TARGET} DEPENDS ${OUTPUT_TARGET}_)

    set(FULL_OUT_PATH ${other_build_dir}/bin/${other_target}.stripped)

    configure_file(${CROSSCMAKE_DIR}/cross_cmake.depfile ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}.d)

    add_custom_command(
        DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_TARGET}.d
        OUTPUT ${FULL_OUT_PATH} ${OUTPUT_TARGET}_
        COMMAND ${CMAKE_COMMAND} --build ${other_build_dir} --target ${other_target}
        COMMENT "Generating ${other_target} in ${other_build_dir}"
        USES_TERMINAL
    )

    add_dependencies(${target} ${OUTPUT_TARGET})
endfunction()