project(tos CXX ASM)

add_library(lx106_impl esp_impl.cpp tos_arch.hpp user_config.h native.s lx106_missing.hpp)
target_include_directories(lx106_impl PRIVATE "../../")
target_include_directories(lx106_impl PUBLIC ".")
target_compile_options(lx106_impl PUBLIC -std=gnu++14)
target_link_libraries(lx106_impl PUBLIC "-Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc")
target_include_directories(lx106_impl PUBLIC "/home/fatih/esp-open-sdk/sdk/include")
target_link_libraries(lx106_impl PUBLIC tos_ft)