add_executable(testcrypto test_crypto.c)
target_link_libraries(testcrypto PRIVATE BearSSL::BearSSL BearSSL::Private)

add_executable(testspeed test_speed.c)
target_link_libraries(testspeed PRIVATE BearSSL::BearSSL BearSSL::Private)

add_executable(testx509 test_x509.c)
target_link_libraries(testx509 PRIVATE BearSSL::BearSSL BearSSL::Private)
