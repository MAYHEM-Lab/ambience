add_subdirectory(l4_iot_disco)
add_subdirectory(iot_epd)
add_subdirectory(nucleo_l412)
add_subdirectory(toblerone)

if (TOS_BOARD)
    add_library(tos_board board.cpp)
    target_include_directories(tos_board PUBLIC ".")
    target_include_directories(tos_board PUBLIC ${CMAKE_BINARY_DIR}/gen/include)
    target_link_libraries(tos_board PUBLIC "${TOS_BOARD}_board")
    target_compile_features(tos_board PUBLIC cxx_std_17)

    configure_file(board.hpp.in ${CMAKE_BINARY_DIR}/gen/include/tos/board.hpp)
endif()
