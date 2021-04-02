message(STATUS "Have address space: ${TOS_ARCH_HAVE_ADDRESS_SPACE}")

if (NOT TOS_ARCH_HAVE_ADDRESS_SPACE)
    return()
endif()

add_library(tos_address_space
    include/tos/address_space.hpp
    src/address_space.cpp
    include/tos/physical_memory_backing.hpp
    src/physical_memory_backing.cpp
    include/tos/backing_object.hpp
    include/tos/mapping.hpp
)
target_link_libraries(tos_address_space PUBLIC tos_arch_core)
target_include_directories(tos_address_space PUBLIC include)