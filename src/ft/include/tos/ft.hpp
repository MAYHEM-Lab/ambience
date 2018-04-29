#pragma once

#include <stdint.h>

namespace tos
{
  using entry_t = void (*)();
  void launch(entry_t) __attribute__((visibility("default")));
  void schedule();

  namespace this_thread
  {
    int get_id();
    void yield();
    void exit(void* res = nullptr);
  }

  uint8_t runnable_count();
}

