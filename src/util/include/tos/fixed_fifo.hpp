//
// Created by fatih on 6/27/18.
//

#pragma once

#include <stddef.h>
#include <tos/ring_buf.hpp>
#include <memory>
#include <new>
#include <tos/interrupt.hpp>
#include <tos/utility.hpp>
#include <tos/sync_ring_buf.hpp>

namespace tos {
  template<class T, size_t Len, class RingBufT = sync_ring_buf>
  class fixed_fifo {
  public:
    void push(T t);

    T pop();

    size_t size() const { return m_rb.size(); }

    size_t capacity() const { return Len; }

  private:

    using TT = std::aligned_storage_t<sizeof(T), alignof(T)>;
    union elem {
      struct {
      } empty;
      TT t;
    };
    elem m_buf[Len];
    RingBufT m_rb{Len};
  };
}

// IMPL

namespace tos {
  template<class T, size_t Len, class RingBufT>
  void fixed_fifo<T, Len, RingBufT>::push(T t) {
    auto i = m_rb.push();
    // push can block, so we disable interrupts after we get the index
    int_guard ig;
    std::destroy_at(&(m_buf[i].empty));
    new(&m_buf[i].t) T(std::move(t));
  }

  template<class T, size_t Len, class RingBufT>
  T fixed_fifo<T, Len, RingBufT>::pop() {
    auto i = m_rb.pop();
    // pop can block, so we disable interrupts after we get the index
    int_guard ig;
    auto ptr = reinterpret_cast<T *>(&(m_buf[i].t));
    auto res = std::move(*ptr);
    std::destroy_at(ptr);
    new(&m_buf[i].empty) decltype(m_buf[0].empty)();
    return res;
  }
}
