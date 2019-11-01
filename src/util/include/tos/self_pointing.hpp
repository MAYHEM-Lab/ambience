//
// Created by fatih on 10/22/19.
//

#pragma once

namespace tos {
/**
 * Due to a lack of smart references in C++, drivers must be accessed
 * as if they are pointers in generic code so that we can allow
 * extensibility on the driver implementer's side. However, a pure
 * pointer based approach wouldn't be efficient in case drivers
 * use value semantics.
 *
 * A common approach to the problem is to just overload the pointer
 * operators to return this to allow for an object to be used as a
 * pointer to itself.
 *
 * This class template simplifies that problem.
 *
 * @tparam DriverT type of the driver
 */
template<class DriverT>
struct self_pointing {
    DriverT* operator->() {
        return static_cast<DriverT*>(this);
    }
    const DriverT* operator->() const {
        return static_cast<const DriverT*>(this);
    }
    DriverT& operator*() {
        return *static_cast<DriverT*>(this);
    }
    const DriverT& operator*() const {
        return *static_cast<const DriverT*>(this);
    }
};
} // namespace tos