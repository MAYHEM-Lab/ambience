project(tos)

add_subdirectory(arch)
message(STATUS "TOS_ARCH_${TOS_ARCH_NAME}")

add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(ft)
add_subdirectory(drivers)
