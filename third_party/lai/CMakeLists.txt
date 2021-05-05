add_library(lai "")
target_include_directories(lai PUBLIC src/include)
target_sources(lai PRIVATE
    src/core/error.c
    src/core/eval.c
    src/core/exec.c
    src/core/exec-operand.c
    src/core/libc.c
    src/core/ns.c
    src/core/object.c
    src/core/opregion.c
    src/core/os_methods.c
    src/core/variable.c
    src/core/vsnprintf.c
    src/helpers/pc-bios.c
    src/helpers/pci.c
    src/helpers/resource.c
    src/helpers/sci.c
    src/helpers/pm.c
    src/drivers/ec.c
    src/drivers/timer.c
    host.cpp
)
target_link_libraries(lai PUBLIC tos_debug_core tos_ft)