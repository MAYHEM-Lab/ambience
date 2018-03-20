#include "tos/semaphore.hpp"
#include "tos/atomic.hpp"

namespace ft
{
  void semaphore::up() noexcept
  {
    ++m_count;
    m_wait.signal_one();
  }
  void semaphore::down() noexcept
  {
    --m_count;
    if (m_count < 0)
    {
      m_wait.wait();
    }
  }
}

