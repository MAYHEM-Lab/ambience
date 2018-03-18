#include "tos/semaphore.hpp"
#include "tos/atomic.hpp"

namespace ft
{
  void semaphore::up()
  {
    ++m_count;
    m_wait.signal_one();
  }
  void semaphore::down()
  {
    --m_count;
    if (m_count < 0)
    {
      m_wait.wait();
    }
  }
}

