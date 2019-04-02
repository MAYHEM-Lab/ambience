//
// Created by fatih on 8/1/18.
//

#include <kt/kt.h>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <memory>
#include <new>

#include <stdlib.h>

extern "C"
{
void *kt_fork(void *(*func)(void *), void *arg) {
    return &tos::launch(tos::alloc_stack, func, arg);
}

void kt_exit() {
    tos::this_thread::exit();
}

void *kt_self() {
    return reinterpret_cast<void *>(tos::this_thread::get_id().id);
}

void kt_yield() {
    tos::this_thread::yield();
}

kt_sem make_kt_sem(int initval) {
    auto ptr = malloc(sizeof(tos::semaphore));
    return new (ptr) tos::semaphore(initval);
}

void kill_kt_sem(kt_sem ksem) {
    auto sem = static_cast<tos::semaphore*>(ksem);
    std::destroy_at(sem);
    free(sem);
}

void P_kt_sem(kt_sem ksem) {
    auto sem = static_cast<tos::semaphore*>(ksem);
    sem->down();
}

void V_kt_sem(kt_sem ksem) {
    auto sem = static_cast<tos::semaphore*>(ksem);
    sem->up();
}

int kt_getval(kt_sem ksem) {
    auto sem = static_cast<tos::semaphore*>(ksem);
    return get_count(*sem);
}
}