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

    explicit semaphore(int8_t n) noexcept : m_count(n) {}
  private:
    tos::atomic<int8_t> m_count;
    waitable m_wait;
  };
}

