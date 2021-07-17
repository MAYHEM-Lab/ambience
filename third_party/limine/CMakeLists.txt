set(LIMINE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

function(add_iso target source)
    find_program(XORRISO_BIN xorriso)
    find_program(LIMINE_INSTALL_BIN limine-install HINTS ${LIMINE_DIR}/binaries)

    if (XORRISO_BIN MATCHES "NOTFOUND")
        message(WARNING "xorriso was not found, will not generate a bootable ISO")
        return()
    endif ()

    if (LIMINE_INSTALL_BIN MATCHES "NOTFOUND")
        message(WARNING "limine-install was not found, will not generate a bootable ISO")
        return()
    endif ()

    set(ISO_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
    set(FULL_ISO_PATH ${CMAKE_BINARY_DIR}/iso/${target}.iso)

    message(STATUS "Found xorriso(${XORRISO_BIN}) and limine-install(${LIMINE_INSTALL_BIN}), will generate a bootable ISO ${target} from ${source}")
    file(MAKE_DIRECTORY ${ISO_BASE_DIR})

    configure_file(${LIMINE_DIR}/limine.cfg.in ${ISO_BASE_DIR}/limine.cfg)

    file(COPY ${LIMINE_DIR}/binaries/limine-eltorito-efi.bin DESTINATION ${ISO_BASE_DIR})
    file(COPY ${LIMINE_DIR}/binaries/limine-cd.bin DESTINATION ${ISO_BASE_DIR})
    file(COPY ${LIMINE_DIR}/binaries/limine.sys DESTINATION ${ISO_BASE_DIR})

    add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${source}>.stripped ${ISO_BASE_DIR}/myos.elf
            OUTPUT ${ISO_BASE_DIR}/myos.elf
            DEPENDS $<TARGET_FILE:${source}>
    )

    add_custom_command(
            COMMAND ${XORRISO_BIN} -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-eltorito-efi.bin -efi-boot-part --efi-boot-image --protective-msdos-label ${ISO_BASE_DIR} -o ${FULL_ISO_PATH}
            COMMAND ${LIMINE_INSTALL_BIN} ${FULL_ISO_PATH}
            DEPENDS ${ISO_BASE_DIR}/myos.elf ${ISO_BASE_DIR}/limine.cfg
            OUTPUT ${FULL_ISO_PATH}
    )

    add_custom_target(${target} DEPENDS ${FULL_ISO_PATH})
endfunction()