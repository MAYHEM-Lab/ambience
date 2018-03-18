#pragma once

#include "waitable.hpp"

namespace ft
{
  class semaphore
  {
  public:
    void up();
    void down();

    semaphore(int n) : m_count(n) {}
  private:
    int m_count;
    waitable m_wait;
  };
}

