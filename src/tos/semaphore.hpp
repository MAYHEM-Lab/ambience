#pragma once

#include "waitable.hpp"
#include "atomic.hpp"

namespace ft
{
  class semaphore
  {
  public:
    void up();
    void down();

    semaphore(int n) : m_count(n) {}
  private:
    tos::atomic<int> m_count;
    waitable m_wait;
  };
}

