include(Tosdoc)

add_tosdoc(
    arm-common.md
    nrf52_dk.md
    stm32f1-nucleo.md
    troubleshooting.adoc
    Building.adoc
    contexts.adoc)

add_subdirectory(cpp)
add_subdirectory(cpus)
add_subdirectory(general)
add_subdirectory(testing)