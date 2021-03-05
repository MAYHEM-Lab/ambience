#pragma once

#include <memory>
#include <tos/virtio/transport.hpp>
#include <tos/x86_64/pci.hpp>

namespace tos::virtio {
std::unique_ptr<transport> make_x86_pci_transport(x86_64::pci::device&& dev);
}