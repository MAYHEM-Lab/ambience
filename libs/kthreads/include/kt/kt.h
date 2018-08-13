//
// Created by fatih on 8/1/18.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void *kt_fork(void *(*func)(void *), void *arg);

void kt_exit();

void *kt_self();

void kt_yield();

typedef void* kt_sem;

kt_sem make_kt_sem(int initval);
void kill_kt_sem(kt_sem ksem);
void P_kt_sem(kt_sem ksem);
void V_kt_sem(kt_sem ksem);
int kt_getval(kt_sem s);

#ifdef __cplusplus
}
#endif