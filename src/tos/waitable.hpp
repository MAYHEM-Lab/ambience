#pragma once

#include "thread_info.hpp"

namespace tos
{
  struct waitable
  {
    void wait();
    void add(thread_info& t);
    void signal_all();
    void signal_one();
    
  private:
    intrusive_list<thread_info> m_waiters;
  };
}
