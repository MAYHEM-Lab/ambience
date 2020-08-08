add_library(bluenrgms "")
target_sources(bluenrgms PRIVATE
    src/hci/hci_le.c
    adapter/ble_list.cpp
    src/hci/controller/bluenrg_gap_aci.c
    src/hci/controller/bluenrg_gatt_aci.c
    src/hci/controller/bluenrg_hal_aci.c
    src/hci/controller/bluenrg_IFR.c
    src/hci/controller/bluenrg_l2cap_aci.c
    src/hci/controller/bluenrg_updater_aci.c
    src/hci/controller/bluenrg_utils_small.c
    adapter/hci_tl.cpp
)

target_include_directories(bluenrgms PUBLIC src/includes)
target_include_directories(bluenrgms PUBLIC src/utils)
target_include_directories(bluenrgms PUBLIC adapter/include)
target_link_libraries(bluenrgms PUBLIC tos_core)