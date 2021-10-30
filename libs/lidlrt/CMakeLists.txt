add_library(lidl_rt INTERFACE)
target_include_directories(lidl_rt INTERFACE include)
target_compile_features(lidl_rt INTERFACE cxx_std_17)

if (TOS)
    target_link_libraries(lidl_rt INTERFACE tos_util_core)
    target_compile_definitions(lidl_rt INTERFACE TOS)
endif()

function(add_lidl_dependency)
    
endfunction()
