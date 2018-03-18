#include "tos/semaphore.hpp"

#include <util/atomic.h>

namespace ft
{
  void semaphore::up()
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      m_count++;
    }
    m_wait.signal_one();
  }
  void semaphore::down()
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      m_count--;
    }
    if (m_count < 0)
    {
      m_wait.wait();
    }
  }
}

