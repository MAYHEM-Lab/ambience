#include "tos/timer.hpp"

namespace fs
{
  void timer::start(long mill)
  {
    m_millis = mill;
  }
}

