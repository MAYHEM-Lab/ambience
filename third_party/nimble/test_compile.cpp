#include "controller/ble_ll.h"
#include "nimble/nimble_port.h"

void tos_main()
{
    nimble_port_ll_task_func(nullptr);
}