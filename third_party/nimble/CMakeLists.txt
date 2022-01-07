add_library(nimble "")
target_include_directories(nimble PUBLIC src/nimble/transport/ram/include)
target_include_directories(nimble PUBLIC src/nimble/controller/include)
target_include_directories(nimble PUBLIC src/nimble/drivers/nrf52/include)
target_include_directories(nimble PUBLIC src/nimble/include)

target_compile_definitions(nimble PUBLIC NIMBLE_CFG_CONTROLLER=1)

target_sources(nimble PRIVATE
    src/nimble/transport/ram/src/ble_hci_ram.c
    src/nimble/controller/src/ble_ll.c
    src/nimble/controller/src/ble_ll_adv.c
    src/nimble/controller/src/ble_ll_conn.c
    src/nimble/controller/src/ble_ll_conn_hci.c
    src/nimble/controller/src/ble_ll_ctrl.c
    src/nimble/controller/src/ble_ll_dtm.c
    src/nimble/controller/src/ble_ll_hci.c
    src/nimble/controller/src/ble_ll_hci_ev.c
    src/nimble/controller/src/ble_ll_hci_vs.c
    src/nimble/controller/src/ble_ll_iso.c
    src/nimble/controller/src/ble_ll_rand.c
    src/nimble/controller/src/ble_ll_resolv.c
    src/nimble/controller/src/ble_ll_rfmgmt.c
    src/nimble/controller/src/ble_ll_scan.c
    src/nimble/controller/src/ble_ll_scan_aux.c
    src/nimble/controller/src/ble_ll_sched.c
    src/nimble/controller/src/ble_ll_supp_cmd.c
    src/nimble/controller/src/ble_ll_sync.c
    src/nimble/controller/src/ble_ll_trace.c
    src/nimble/controller/src/ble_ll_utils.c
    src/nimble/controller/src/ble_ll_whitelist.c
    src/nimble/drivers/nrf52/src/ble_hw.c
    src/nimble/drivers/nrf52/src/ble_phy.c
    src/nimble/drivers/nrf52/src/ble_phy_trace.c

    src/porting/nimble/src/os_mempool.c
    src/porting/nimble/src/os_mbuf.c
    src/porting/nimble/src/endian.c
)

target_link_libraries(nimble PUBLIC tos::libc nrf5-sdk)

# target_include_directories(nimble PUBLIC include)
target_include_directories(nimble PUBLIC src/porting/nimble/include)

add_subdirectory(porting/npl/tos)

add_executable(nimble_test_compile test_compile.cpp)
target_link_libraries(nimble_test_compile PRIVATE nimble)