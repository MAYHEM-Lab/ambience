#pragma once

namespace tos {
    void make_runnable(struct thread_info *t);

    void wait_yield();
}

