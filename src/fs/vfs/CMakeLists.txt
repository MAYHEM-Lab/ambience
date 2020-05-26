add_library(tos_vfs "")
target_sources(tos_vfs PRIVATE
        include/tos/vfs/common.hpp
        include/tos/vfs/directory.hpp
        include/tos/vfs/file.hpp
        include/tos/vfs/filesystem.hpp

        src/directory.cpp
        src/file.cpp
        src/filesystem.cpp
        src/vfs.cpp
        src/node.cpp)
target_link_libraries(tos_vfs PUBLIC tos_util_core)
target_include_directories(tos_vfs PUBLIC include)