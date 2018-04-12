#pragma once
#include "intrusive_list.hpp"
#include <setjmp.h>

namespace ft
{
  struct thread_info 
    : public list_node<thread_info>
  {
    char id;
    void* stack;
    void (*entry)();
    jmp_buf context;
  };

  thread_info* self();
}

