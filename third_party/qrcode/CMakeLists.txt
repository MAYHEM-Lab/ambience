add_library(qrcode src/src/qrcode.c)
target_include_directories(qrcode PUBLIC src/src)
target_link_libraries(qrcode PUBLIC tos::libc)

add_library(qrcode_gui
        gui/include/tos/gui/elements/extra/qrcode.hpp
        gui/qrcode.cpp)
target_link_libraries(qrcode_gui PUBLIC tos_gui)
target_link_libraries(qrcode_gui PRIVATE qrcode)
target_include_directories(qrcode_gui PUBLIC gui/include)