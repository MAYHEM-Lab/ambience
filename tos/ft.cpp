#include <setjmp.h>
#include "ft.hpp"
#include "thread_info.hpp"
#include "scheduler.hpp"

namespace ft
{
  namespace impl
  {
    void set_stack_ptr(void* ptr);
    void power_down();
  }
}

namespace ft
{
  intrusive_list<thread_info> run_queue;
  
  thread_info* cur_thread = nullptr;
  jmp_buf global_one;
  
  namespace this_thread
  {
    void yield()
    {
      if (setjmp(cur_thread->buffer) == 0)
      {
        // yielded
        make_runnable(cur_thread);
        longjmp(global_one, 1);
      }
      else {
        // We're back to this thread, upon returning from
        // yield, control will be back to the thread
      }
    }
    
    int get_id()
    {
      if (!cur_thread) return -1;
      return cur_thread->id;
    }
  }

  void wait_yield()
  {
    if (setjmp(cur_thread->buffer) == 0)
    {
      // yielded
      longjmp(global_one, 1);
    }
    else {
      // We're back to this thread, upon returning from
      // yield, control will be back to the thread
    }
  }

  thread_info* self()
  {
    return cur_thread;
  }
  
  thread_info** threads = nullptr;
  int num_threads = 0;
  int capacity = 0;
  
  void start(void (*t_start)())
  {
    int index;
    if (capacity == num_threads)
    {
      auto new_one = new thread_info*[capacity + 1];
      for (int i = 0; i < num_threads; ++i)
      {
        new_one[i] = threads[i];
      }
      delete threads;
      threads = new_one;
      capacity++;
      index = num_threads;
    }
    else
    {
      for (auto t = threads; t != threads + capacity; ++t)
      {
        if (t == nullptr)
        {
          index = t - threads;
          break;
        }
      }
    }

    constexpr auto stack_size = 256;

    auto thread = new thread_info();
    num_threads++;
    thread->stack = new char[stack_size];
    thread->entry = t_start;
    thread->id = num_threads;
    threads[index] = thread;
    
    run_queue.push_back(*thread);
  
    if (setjmp(thread->buffer) == 0)
    {
      //done
      return;
    }
  
    // this is the actual entry point of the thread
    // will be called when scheduled
    impl::set_stack_ptr(cur_thread->stack + stack_size);
    cur_thread->entry();
    this_thread::exit(nullptr);
  }

  void this_thread::exit(void*)
  {
    delete cur_thread->stack;
    cur_thread->stack = nullptr;
    delete cur_thread;
    num_threads--;
    wait_yield();
  }
  
  void schedule()
  {
    if (run_queue.empty()){
      impl::power_down();
    }
  
    if (setjmp(global_one) == 0)
    {
      // run it
      cur_thread = &run_queue.front();
      run_queue.pop_front();
      longjmp(cur_thread->buffer, 1);
    }
    else
    {
      cur_thread = nullptr;
      // yielded
    }
  }

  void make_runnable(thread_info* t)
  {
    run_queue.push_back(*t);
  }
}


