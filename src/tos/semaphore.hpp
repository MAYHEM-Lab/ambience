#pragma once

#include "waitable.hpp"
#include "atomic.hpp"

namespace ft
{
  class semaphore
  {
  public:
    void up() noexcept;
    void down() noexcept;

    semaphore(int n) noexcept : m_count(n) {}
  private:
    tos::atomic<int> m_count;
    waitable m_wait;
  };
}

