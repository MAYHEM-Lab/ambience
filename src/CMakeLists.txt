project(tos)

add_subdirectory(platform)
message(STATUS "TOS_ARCH_${TOS_ARCH_NAME}")
set(TOS_ARCH_NAME ${TOS_ARCH_NAME} PARENT_SCOPE)

add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(ft)
add_subdirectory(drivers)
