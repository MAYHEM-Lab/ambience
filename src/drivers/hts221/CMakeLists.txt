add_library(tos_device_hts221 hts221.cpp include/tos/device/detail/hts221_registers.hpp)
target_link_libraries(tos_device_hts221 PUBLIC tos::drivers)
target_include_directories(tos_device_hts221 PUBLIC include)