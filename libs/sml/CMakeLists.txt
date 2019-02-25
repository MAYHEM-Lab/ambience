

add_library(boost_sml include/boost/sml.hpp stub.cpp)
tos_install(boost_sml ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Boost::sml ALIAS boost_sml)