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
    explicit tracked_driver(int num) { assign(num, *static_cast<DriverT*>(this)); }

    template <int N>
    explicit tracked_driver(std::integral_constant<int, N>) { assign<N>(*static_cast<DriverT*>(this)); }

    template <int Num>
    static void assign(DriverT& d)
    {
        m_drivers[Num] = get_ptr(d);
    }

    static void assign(int num, DriverT& d)
    {
        m_drivers[num] = get_ptr(d);
    }

    template <int Num>
    static const track_ptr<DriverT>& get()
    {
        return m_drivers[Num];
    }

    static const track_ptr<DriverT>& get(int num)
    {
        return m_drivers[num];
    }

private:

    static track_ptr<DriverT> m_drivers[DriverNum];
};

template <class DriverT, int DriverNum>
track_ptr<DriverT> tracked_driver<DriverT, DriverNum>::m_drivers[DriverNum];
}