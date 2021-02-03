#pragma once

#include <cstdint>

namespace tos::pci {
enum classes
{
    unclassified = 0,
    storage = 1,
    network = 2,
    display = 3,
    multimedia = 4,
    memory = 5
};
}
