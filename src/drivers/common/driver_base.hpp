//
// Created by fatih on 12/1/18.
//

#pragma once

namespace tos
{
template <class DriverT>
struct self_pointing
{
    DriverT*operator->(){ return static_cast<DriverT*>(this); }
    DriverT&operator*() { return *static_cast<DriverT*>(this); }
};

template <class DriverT, int DriverNum = 1>
struct tracked_driver
{
public:
    explicit tracked_driver(int num) {
        m_which = &instances[num];
        assign();
    }

    tracked_driver(tracked_driver&& rhs) : m_which(rhs.m_which)
    {
        rhs.m_which = nullptr;
        assign();
    }

    static DriverT* get(int num)
    {
        return instances[num];
    }

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
}