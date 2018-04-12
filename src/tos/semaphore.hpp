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

    semaphore(uint8_t n) noexcept : m_count(n) {}
  private:
    tos::atomic<uint8_t> m_count;
    waitable m_wait;
  };
}

