function(add_service serv_name interface)
    add_library(${serv_name} ${ARGN})
    #target_link_libraries(${serv_name} PRIVATE --relocatable -einit)
    target_link_libraries(${serv_name} PUBLIC ${interface}_schema)
endfunction()

add_subdirectory(interfaces)
add_subdirectory(impls)