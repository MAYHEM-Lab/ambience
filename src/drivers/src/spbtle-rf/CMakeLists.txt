
add_library(spbtle-rf
        #beacon_service.cpp
        #beacon_service.h
        ble_clock.h
        ble_list.h
        ble_status.h
        bluenrg_aci_const.h
        bluenrg_aci.h
        bluenrg_gap_aci.h
        bluenrg_gap.h
        bluenrg_gatt_aci.h
        bluenrg_gatt_server.h
        bluenrg_hal_aci.h
        bluenrg_interface.c
        bluenrg_interface.h
        bluenrg_l2cap_aci.h
        bluenrg_updater_aci.h
        bluenrg_utils.h
        compiler.h
        connection_config.h
        debug.h
        #eddystone_beacon.c
        #eddystone_beacon.h
        gp_timer.h
        hal.h
        hal_types.h
        hci_const.h
        hci.h
        hci_le.h
        link_layer.h
        osal.h
        #sensor_service.cpp
        #sensor_service.h
        sm.h
        SPBTLE_RF.cpp
        SPBTLE_RF.h
        stm32_bluenrg_ble.h
        STM32_BlueNRG/SimpleBlueNRG_HCI/utils/gp_timer.cpp
        STM32_BlueNRG/SimpleBlueNRG_HCI/utils/ble_list.cpp
        STM32_BlueNRG/SimpleBlueNRG_HCI/utils/osal.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/hci_le.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_utils.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_IFR.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_l2cap_aci.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_updater_aci.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_hal_aci.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_gap_aci.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/controller/bluenrg_gatt_aci.c
        STM32_BlueNRG/SimpleBlueNRG_HCI/hci/hci.cpp

        spbtle_rf.inl)
target_include_directories(spbtle-rf PUBLIC ".")
target_link_libraries(spbtle-rf PUBLIC tos_core tos_ft tos_alarm arch_drivers range-v3)
target_compile_features(spbtle-rf PUBLIC cxx_std_17)
