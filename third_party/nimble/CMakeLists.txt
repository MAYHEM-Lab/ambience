add_library(nimble "")
target_include_directories(nimble PUBLIC src/nimble/transport/ram/include)
target_include_directories(nimble PUBLIC src/nimble/controller/include)
target_include_directories(nimble PUBLIC src/nimble/drivers/nrf52/include)
target_include_directories(nimble PUBLIC src/nimble/include)
target_include_directories(nimble PUBLIC src/nimble/host/include)
target_include_directories(nimble PUBLIC src/nimble/host/util/include)
target_include_directories(nimble PUBLIC src/nimble/host/services/gap/include)
target_include_directories(nimble PUBLIC src/nimble/host/services/gatt/include)

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

    src/nimble/host/src/ble_att.c
    src/nimble/host/src/ble_att_clt.c
    src/nimble/host/src/ble_att_cmd.c
    src/nimble/host/src/ble_att_svr.c
    src/nimble/host/src/ble_eddystone.c
    src/nimble/host/src/ble_gap.c
    src/nimble/host/src/ble_gattc.c
    src/nimble/host/src/ble_gatts.c
    src/nimble/host/src/ble_gatts_lcl.c
    src/nimble/host/src/ble_hs.c
    src/nimble/host/src/ble_hs_adv.c
    src/nimble/host/src/ble_hs_atomic.c
    src/nimble/host/src/ble_hs_cfg.c
    src/nimble/host/src/ble_hs_conn.c
    src/nimble/host/src/ble_hs_flow.c
    src/nimble/host/src/ble_hs_hci.c
    src/nimble/host/src/ble_hs_hci_cmd.c
    src/nimble/host/src/ble_hs_hci_evt.c
    src/nimble/host/src/ble_hs_hci_util.c
    src/nimble/host/src/ble_hs_id.c
    src/nimble/host/src/ble_hs_log.c
    src/nimble/host/src/ble_hs_mbuf.c
    src/nimble/host/src/ble_hs_misc.c
    src/nimble/host/src/ble_hs_mqueue.c
    src/nimble/host/src/ble_hs_periodic_sync.c
    src/nimble/host/src/ble_hs_pvcy.c
    src/nimble/host/src/ble_hs_shutdown.c
    src/nimble/host/src/ble_hs_startup.c
    src/nimble/host/src/ble_hs_stop.c
    src/nimble/host/src/ble_ibeacon.c
    src/nimble/host/src/ble_l2cap.c
    src/nimble/host/src/ble_l2cap_coc.c
    src/nimble/host/src/ble_l2cap_sig.c
    src/nimble/host/src/ble_l2cap_sig_cmd.c
    src/nimble/host/src/ble_monitor.c
    src/nimble/host/src/ble_sm.c
    src/nimble/host/src/ble_sm_alg.c
    src/nimble/host/src/ble_sm_cmd.c
    src/nimble/host/src/ble_sm_lgcy.c
    src/nimble/host/src/ble_sm_sc.c
    src/nimble/host/src/ble_store.c
    src/nimble/host/src/ble_store_util.c
    src/nimble/host/src/ble_uuid.c

    src/nimble/host/util/src/addr.c

    src/nimble/host/services/gap/src/ble_svc_gap.c

    src/nimble/drivers/nrf52/src/ble_hw.c
    src/nimble/drivers/nrf52/src/ble_phy.c
    src/nimble/drivers/nrf52/src/ble_phy_trace.c

    src/porting/nimble/src/os_mempool.c
    src/porting/nimble/src/os_mbuf.c
    src/porting/nimble/src/endian.c
    src/porting/nimble/src/os_cputime.c
    src/porting/nimble/src/os_cputime_pwr2.c
    src/porting/nimble/src/hal_timer.c
    src/porting/nimble/src/nimble_port.c
    src/porting/nimble/src/mem.c
    src/porting/nimble/src/os_msys_init.c
)

# target_compile_options(nimble PRIVATE -fno-lto -fno-whole-program-vtables)
target_link_libraries(nimble PUBLIC tos::libc nrf5-sdk)

# target_include_directories(nimble PUBLIC include)
target_include_directories(nimble PUBLIC src/porting/nimble/include)

add_subdirectory(porting/npl/tos)

add_executable(nimble_test_compile test_compile.cpp)
target_link_libraries(nimble_test_compile PRIVATE nimble)