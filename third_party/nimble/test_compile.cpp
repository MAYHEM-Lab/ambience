#include "controller/ble_ll.h"
#include "nimble/nimble_port.h"

extern "C" void ble_hci_ram_init();

void tos_main()
{
    nimble_port_init();
    ble_hci_ram_init();
    ble_ll_init();
}