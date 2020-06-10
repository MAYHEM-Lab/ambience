#pragma once

#include <tos/device/spbtlerf/gatt.hpp>
#include <tos/function_ref.hpp>
#include <memory>

namespace tos::device::spbtle {
struct evt_handler_impl;
/**
 * An instance of this class is used to handle events raised from the driver.
 */
class hci_evt_handler {
public:
    hci_evt_handler();
    ~hci_evt_handler();

    void operator()(void* packet);

    void register_service(gatt_service& serv);

    void remove_service(gatt_service& serv);
private:
    std::unique_ptr<evt_handler_impl> m_impl;
};
}