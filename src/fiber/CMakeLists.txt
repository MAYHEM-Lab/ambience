add_library(tos_fiber fiber.cpp include/tos/fiber/awaitable_fiber.hpp)
target_link_libraries(tos_fiber PUBLIC tos_pcontext)
target_include_directories(tos_fiber PUBLIC include)

add_library(tos_fiber_single_thread single_thread.cpp)
target_link_libraries(tos_fiber_single_thread PUBLIC tos_fiber)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()