file(GLOB ARCH_FILES "*.S")

target_sources(
        picolibc
        PRIVATE
        ${ARCH_FILES}
)

file(GLOB ARCH_FILES "*.c")

target_sources(
        picolibc
        PRIVATE
        ${ARCH_FILES}
)

target_include_directories(picolibc PRIVATE .)