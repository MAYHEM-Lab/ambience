set(ADDISO_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_iso target source)
    find_program(MKRESCUE_BIN grub-mkrescue)

    if (MKRESCUE_BIN MATCHES "NOTFOUND")
        message(WARNING "grub-mkrescue was not found, will not generate a bootable ISO")
        return()
    endif()

    set(ISO_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
    set(FULL_ISO_PATH ${CMAKE_BINARY_DIR}/iso/${target}.iso)

    message(STATUS "Found grub-mkrescue(${MKRESCUE_BIN}), will generate a bootable ISO ${target} from ${source}")
    file(MAKE_DIRECTORY ${ISO_BASE_DIR}/boot/grub)

    configure_file(${ADDISO_DIR}/grub.cfg.in ${ISO_BASE_DIR}/boot/grub/grub.cfg)

    add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${source}>.stripped ${ISO_BASE_DIR}/boot/image.elf
            OUTPUT ${ISO_BASE_DIR}/boot/image.elf
            DEPENDS $<TARGET_FILE:${source}>
    )

    add_custom_command(
            COMMAND ${MKRESCUE_BIN} -o ${FULL_ISO_PATH} ${ISO_BASE_DIR}
            DEPENDS ${ISO_BASE_DIR}/boot/image.elf ${ISO_BASE_DIR}/boot/grub/grub.cfg
            OUTPUT ${FULL_ISO_PATH}
    )

    add_custom_target(${target} DEPENDS ${FULL_ISO_PATH})
endfunction()
