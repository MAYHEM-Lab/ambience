include(CMakeParseArguments)
include(TosFunctions)

function(find_gnu_toolchain)
    cmake_parse_arguments(
        TOOLCHAIN # prefix of output variables
        "SET" # list of names of the boolean arguments (only defined ones will be true)
        "TRIPLE" # list of names of mono-valued arguments
        "HINTS" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT TOOLCHAIN_TRIPLE)
        message(FATAL_ERROR "Target triple must be specified for find_toolchain")
    endif()

    set(SEARCH_DIRS ${TOOLCHAIN_HINTS})
    list(APPEND SEARCH_DIRS /opt/x-tools)
    list(APPEND SEARCH_DIRS C:/x-tools)

    SUBDIRLIST(X_TOOLS_SUBS C:/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS C:/x-tools/${path})
    endforeach()

    SUBDIRLIST(X_TOOLS_SUBS /opt/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS /opt/x-tools/${path})
    endforeach()

    find_program(TOOLCHAIN_CXX_COMPILER 
        ${TOOLCHAIN_TRIPLE}-g++
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_C_COMPILER 
        ${TOOLCHAIN_TRIPLE}-gcc
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_SIZE 
        ${TOOLCHAIN_TRIPLE}-size
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)

    find_program(TOOLCHAIN_OBJCOPY 
        ${TOOLCHAIN_TRIPLE}-objcopy
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)

    find_program(TOOLCHAIN_LD 
        ${TOOLCHAIN_TRIPLE}-ld
        HINTS ${SEARCH_DIRS}
        PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_LD ${TOOLCHAIN_LD} PARENT_SCOPE)

    find_program(TOOLCHAIN_AR
            ${TOOLCHAIN_TRIPLE}-gcc-ar
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_AR ${TOOLCHAIN_AR} PARENT_SCOPE)

    find_program(TOOLCHAIN_RANLIB
            ${TOOLCHAIN_TRIPLE}-gcc-ranlib
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_RANLIB ${TOOLCHAIN_RANLIB} PARENT_SCOPE)

    if (TOOLCHAIN_SET)
        set(CMAKE_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)
        set(CMAKE_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)
        set(CMAKE_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)
        set(CMAKE_LD ${TOOLCHAIN_LD} PARENT_SCOPE)
        set(CMAKE_LD "${CMAKE_LD}" CACHE STRING "LD")
        set(CMAKE_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)
        set(CMAKE_OBJCOPY "${CMAKE_OBJCOPY}" CACHE STRING "OBJCOPY")
        set(CMAKE_AR ${TOOLCHAIN_AR} PARENT_SCOPE)
        set(CMAKE_AR "${CMAKE_AR}" CACHE STRING "AR")
        set(CMAKE_RANLIB ${TOOLCHAIN_RANLIB} PARENT_SCOPE)
        set(CMAKE_RANLIB "${CMAKE_RANLIB}" CACHE STRING "RANLIB")
    endif()
endfunction()

function(find_llvm_toolchain)
    cmake_parse_arguments(
            TOOLCHAIN # prefix of output variables
            "SET" # list of names of the boolean arguments (only defined ones will be true)
            "TRIPLE" # list of names of mono-valued arguments
            "HINTS" # list of names of multi-valued arguments (output variables are lists)
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if (NOT TOOLCHAIN_TRIPLE)
        message(FATAL_ERROR "Target triple must be specified for find_toolchain")
    endif()

    set(SEARCH_DIRS ${TOOLCHAIN_HINTS})
    list(APPEND SEARCH_DIRS /opt/x-tools)
    list(APPEND SEARCH_DIRS /opt/llvm)
    list(APPEND SEARCH_DIRS C:/x-tools)

    SUBDIRLIST(X_TOOLS_SUBS C:/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS C:/x-tools/${path})
    endforeach()

    SUBDIRLIST(X_TOOLS_SUBS /opt/x-tools)
    foreach(path ${X_TOOLS_SUBS})
        list(APPEND SEARCH_DIRS /opt/x-tools/${path})
    endforeach()

    find_program(TOOLCHAIN_CXX_COMPILER
            clang++
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_C_COMPILER
            clang
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)

    find_program(TOOLCHAIN_SIZE
            llvm-size
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)

    find_program(TOOLCHAIN_OBJCOPY
            llvm-objcopy
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)

    find_program(TOOLCHAIN_LD
            ld.lld
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_LD ${TOOLCHAIN_LD} PARENT_SCOPE)

    find_program(TOOLCHAIN_WASM_LD
            wasm-ld
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_WASM_LD ${TOOLCHAIN_WASM_LD} PARENT_SCOPE)

    find_program(TOOLCHAIN_AR
            llvm-ar
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_AR ${TOOLCHAIN_AR} PARENT_SCOPE)

    find_program(TOOLCHAIN_RANLIB
            llvm-ranlib
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_RANLIB ${TOOLCHAIN_RANLIB} PARENT_SCOPE)

    find_program(TOOLCHAIN_STRIP
            llvm-strip
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_STRIP ${TOOLCHAIN_STRIP} PARENT_SCOPE)

    find_program(TOOLCHAIN_CLANG_TIDY
            clang-tidy
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_CLANG_TIDY ${TOOLCHAIN_CLANG_TIDY} PARENT_SCOPE)

    find_program(TOOLCHAIN_READELF
            llvm-readelf
            HINTS ${SEARCH_DIRS}
            PATH_SUFFIXES bin ${TOOLCHAIN_TRIPLE}/bin)
    set(TOOLCHAIN_READELF ${TOOLCHAIN_READELF} PARENT_SCOPE)

    if (TOOLCHAIN_SET)
        set(CMAKE_CXX_COMPILER ${TOOLCHAIN_CXX_COMPILER} PARENT_SCOPE)
        set(CMAKE_C_COMPILER ${TOOLCHAIN_C_COMPILER} PARENT_SCOPE)
        set(CMAKE_SIZE ${TOOLCHAIN_SIZE} PARENT_SCOPE)
        set(CMAKE_LD ${TOOLCHAIN_LD} PARENT_SCOPE)
        set(CMAKE_LD "${CMAKE_LD}" CACHE STRING "LD")
        set(CMAKE_OBJCOPY ${TOOLCHAIN_OBJCOPY} PARENT_SCOPE)
        set(CMAKE_OBJCOPY "${CMAKE_OBJCOPY}" CACHE STRING "OBJCOPY")
        set(CMAKE_AR ${TOOLCHAIN_AR} PARENT_SCOPE)
        set(CMAKE_AR "${CMAKE_AR}" CACHE STRING "AR")
        set(CMAKE_RANLIB ${TOOLCHAIN_RANLIB} PARENT_SCOPE)
        set(CMAKE_RANLIB "${CMAKE_RANLIB}" CACHE STRING "RANLIB")
        set(CMAKE_STRIP ${TOOLCHAIN_STRIP} PARENT_SCOPE)
        set(CMAKE_STRIP "${CMAKE_STRIP}" CACHE STRING "STRIP")
        set(CMAKE_CXX_CLANG_TIDY ${TOOLCHAIN_CLANG_TIDY} PARENT_SCOPE)
        set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}" CACHE STRING "clang-tidy")
        #set(CMAKE_C_CLANG_TIDY ${TOOLCHAIN_CLANG_TIDY} PARENT_SCOPE)
        #set(CMAKE_C_CLANG_TIDY "${CMAKE_C_CLANG_TIDY}" CACHE STRING "clang-tidy")
    endif()
endfunction()