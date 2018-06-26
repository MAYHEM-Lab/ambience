project(tos)

add_executable(simple_esp main.cpp)
target_link_libraries(simple_esp PUBLIC tos_core esp82_drivers)
target_link_libraries(simple_esp PUBLIC "-Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc")
