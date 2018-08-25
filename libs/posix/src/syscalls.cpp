//
// Created by fatih on 8/22/18.
//

#include "errno.h"
#include <tos/posix.hpp>

namespace tos
{
    namespace posix
    {
        file_desc* from_fd(int fd)
        {
            return nullptr;
        }
    }
}

#define syscall_ret(x) \
{ \
  if (int(x) < 0) \
    errno = -x;\
  return x; \
}

extern "C"
{
    int _read(int fd, void* to, size_t len)
    {
        auto obj = tos::posix::from_fd(fd);
        if (!obj)
        {
            syscall_ret(-EBADF);
        }

        auto res = obj->read(tos::span<char>{(char*)to, len});

        syscall_ret(res.size());
    }

    struct _reent;
    size_t _read_r(_reent*, int fd, void* to, size_t len)
    {
        return _read(fd, to, len);
    }
}