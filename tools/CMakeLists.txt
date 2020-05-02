add_executable(brssl "")
target_sources(brssl PRIVATE
    brssl.c
    brssl.h
    certs.c
    chain.c
    client.c
    CMakeLists.txt
    errors.c
    files.c
    impl.c
    keys.c
    names.c
    server.c
    skey.c
    sslio.c
    ta.c
    twrch.c
    vector.c
    verify.c
    xmem.c
)
target_link_libraries(brssl PRIVATE BearSSL::BearSSL)