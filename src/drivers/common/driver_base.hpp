//
// Created by fatih on 12/1/18.
//

#pragma once

#include <array>
#include <tos/debug.hpp>

namespace tos
{
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
    template <class DriverT>
    struct self_pointing
    {
        DriverT*operator->(){ return static_cast<DriverT*>(this); }
        DriverT&operator*() { return *static_cast<DriverT*>(this); }
    };

    /**
     * For actual hardware drivers, there's usually a need to locate
     * driver specific data from a global function, usually an interrupt
     * service routine.
     *
     * Tos allows for drivers to be implemented as value types, thus
     * the driver object is actually expected to be moved from stack
     * frame to stack frame. This makes it very difficult for an ISR
     * to do it's job. The conventional idea is to put the data that
     * might be needed from an ISR in a global struct. This feels like
     * a hack, and potentially wastes too much space in case the needed
     * data is large.
     *
     * A solution is to keep a global pointer that is updated every
     * time the driver instance is moved.
     *
     * This template implements the said functionality with support for
     * multi-instance drivers.
     *
     * @tparam DriverT type of the driver
     * @tparam DriverNum number of potential instances
     */
    template <class DriverT, int DriverNum = 1>
    struct tracked_driver
    {
    public:
        explicit tracked_driver(int num) {
            m_which = &instances[num];
            if (*m_which)
            {
                tos::kern::fatal("driver already exists!");
            }
            assign();
        }

        tracked_driver(tracked_driver&& rhs) noexcept
            : m_which(rhs.m_which)
        {
            rhs.m_which = nullptr;
            assign();
        }

        /**
         * Gets the current address of the num'th instance of the
         * driver.
         *
         * Returns nullptr in the case the driver isn't alive at
         * this time.
         *
         * @param num instance number
         * @return pointer to the driver
         */
        static DriverT* get(int num)
        {
            return instances[num];
        }

        ~tracked_driver() { *m_which = nullptr; }

    private:
        void assign()
        {
            *m_which = static_cast<DriverT*>(this);
        }

        DriverT** m_which;
        static std::array<DriverT*, DriverNum> instances;
    };

    template <class DriverT, int DriverNum>
    std::array<DriverT*, DriverNum> tracked_driver<DriverT, DriverNum>::instances {};
} // namespace tos