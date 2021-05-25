project(tos)

add_subdirectory(arch)

add_subdirectory(platform)
message(STATUS "TOS_PLATFORM_${TOS_PLATFORM_NAME}")
set(TOS_PLATFORM_NAME ${TOS_PLATFORM_NAME} PARENT_SCOPE)

add_subdirectory(soc)
add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(pcontext)
add_subdirectory(ft)
add_subdirectory(drivers)
add_subdirectory(sanitizer)