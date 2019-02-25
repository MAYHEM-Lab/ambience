
add_library(zmtp3 include/tos/zmtp/zmtplib.h src/zmtp.cpp)
target_link_libraries(zmtp3 PUBLIC tos_core esp82_drivers)
target_include_directories(zmtp3 PUBLIC "include")