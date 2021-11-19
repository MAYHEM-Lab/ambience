add_library(tos_fiber fiber.cpp)
target_link_libraries(tos_fiber PUBLIC tos_pcontext)
target_include_directories(tos_fiber PUBLIC include)