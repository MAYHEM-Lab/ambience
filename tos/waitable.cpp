#include "waitable.hpp"
#include "scheduler.hpp"

namespace ft
{
  void waitable::wait()
  {
    add(*self());
    wait_yield();
  }
  
  void waitable::add(thread_info& t)
  {
    m_waiters.push_back(t);
  }
  
  void waitable::signal_all()
  {
    while (!m_waiters.empty())
    {
      signal_one();
    }
  }
  
  void waitable::signal_one()
  {
    if (m_waiters.empty()) return;
    auto front = &m_waiters.front();
    m_waiters.pop_front();
    make_runnable(front);    
  }
}

