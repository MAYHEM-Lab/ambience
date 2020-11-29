if (NOT ${TOS_PLATFORM} MATCHES "raspi")
	return()
endif()

cmake_minimum_required(VERSION 3.13)
project(uspi C CXX)

add_library(uspi "")
target_include_directories(uspi PUBLIC include)
target_sources(uspi PRIVATE
	lib/devicenameservice.c
	lib/dwhcidevice.cpp
	lib/dwhciframeschednper.c
	lib/dwhciframeschednsplit.c
	lib/dwhciframeschedper.c
	lib/dwhciregister.c
	lib/dwhcirootport.c
	lib/dwhcixferstagedata.c
	lib/keymap.c
	lib/lan7800.c
	lib/macaddress.c
	lib/smsc951x.c
	lib/string.c
	lib/synchronize.cpp
	lib/usbconfigparser.c
	lib/usbdevice.c
	lib/usbdevicefactory.c
	lib/usbendpoint.c
	lib/usbfunction.c
	lib/usbgamepad.c
	lib/usbkeyboard.c
	lib/usbmassdevice.c
	lib/usbmidi.c
	lib/usbmouse.c
	lib/usbrequest.c
	lib/usbstandardhub.c
	lib/usbstring.c
	lib/util.c
)
target_compile_definitions(uspi PUBLIC AARCH64=1)
target_link_libraries(uspi PRIVATE tos_ft)