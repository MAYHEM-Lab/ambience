//
// Created by fatih on 1/9/19.
//

#include <cstddef>
#include <cstdlib>
#include <csignal>

namespace tos::platform {
void force_reset() {
#if defined(SIGTRAP)
    raise(SIGTRAP);
#endif
    exit(1);
}
}