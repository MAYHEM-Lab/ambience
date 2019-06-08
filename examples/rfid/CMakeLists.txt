add_executable(rfid main.cpp)
target_link_libraries(rfid PUBLIC tos::core arch_drivers tos::print)