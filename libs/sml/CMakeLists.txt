

add_library(boost_sml include/boost/sml.hpp stub.cpp)
target_include_directories(boost_sml PUBLIC "include")

add_library(Boost::sml ALIAS boost_sml)