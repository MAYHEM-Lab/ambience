add_executable(client_basic client_basic.c)
target_link_libraries(client_basic PRIVATE BearSSL::BearSSL)

add_executable(server_basic server_basic.c)
target_link_libraries(server_basic PRIVATE BearSSL::BearSSL)
