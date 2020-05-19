#pragma once

#include "component.hpp"

#include <tos/memory/polymorphic_allocator.hpp>

namespace tos {
struct allocator_component : id_component<1> {
    explicit allocator_component(memory::polymorphic_allocator& alloc)
        : allocator{&alloc} {
    }
    memory::polymorphic_allocator* allocator;
};
} // namespace tos