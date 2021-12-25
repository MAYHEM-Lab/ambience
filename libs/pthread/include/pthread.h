#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/errno.h>

#include <tos/pthread/thread.h>
#include <tos/pthread/semaphore.h>
#include <tos/pthread/mutex.h>
#include <tos/pthread/condvar.h>
#include <tos/pthread/rwlock.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef void* pthread_t;

struct pthread_attr_t {
    size_t stack_size;
    void* stack_addr;
};

// Mutex
struct pthread_mutex_t {
    _Alignas(TOS_MUTEX_ALIGNMENT) char mutex_buffer[TOS_MUTEX_SIZE];
};

#define PTHREAD_MUTEX_INITIALIZER { { } }

struct pthread_mutexattr_t {
    int protocol;
    int pshared;
    int type;
};

// RWLock

struct pthread_rwlock_t {
    _Alignas(TOS_RW_ALIGNMENT) char rwlock_buffer[TOS_RW_SIZE];
};
#define PTHREAD_RWLOCK_INITIALIZER {{}}

typedef void* pthread_rwlockattr_t;

struct pthread_once_t {};
#define PTHREAD_ONCE_INIT {}

struct pthread_key_t {};

enum pthread_mutex_type
{
    PTHREAD_MUTEX_RECURSIVE
};

/* FUNCTIONS */

// Pthread
int pthread_attr_init(pthread_attr_t* attr);
int pthread_attr_destroy(pthread_attr_t* attr);
int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stack_size);
int pthread_attr_getstacksize(const pthread_attr_t* attr, size_t* stack_size);
int pthread_attr_setstackaddr(pthread_attr_t* attr, void* stack_addr);
int pthread_attr_getstackaddr(const pthread_attr_t* attr, void** stack_addr);
int pthread_create(pthread_t* thread,
                   const pthread_attr_t* attr,
                   void* (*start_routine)(void*),
                   void* arg);
int pthread_join(pthread_t thread, void** value_ptr);
int pthread_detach(pthread_t);
int pthread_equal(pthread_t, pthread_t);
void pthread_exit(void*);
pthread_t pthread_self(void);

// Mutex

int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_setprioceiling(pthread_mutex_t* mutex,
                                 int prioceiling,
                                 int* old_ceiling);
int pthread_mutex_getprioceiling(const pthread_mutex_t* mutex, int* prioceiling);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);

int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t* attr, int* protocol);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t* attr, int* pshared);
int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t* attr, int prioceiling);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t* attr, int* prioceiling);
int pthread_mutexattr_gettype(const pthread_mutexattr_t* attr, int* type);
int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type);
int pthread_mutexattr_init(pthread_mutexattr_t* attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t* attr);

// Condition Variable

int pthread_cond_signal(pthread_cond_t* cond);
int pthread_cond_broadcast(pthread_cond_t* cond);
int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr);
int pthread_cond_destroy(pthread_cond_t* cond);
int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);

// Not implemented!
int pthread_cond_timedwait(pthread_cond_t* cond,
                           pthread_mutex_t* mutex,
                           const struct timespec* abstime);

int pthread_condattr_init(pthread_condattr_t* attr);
int pthread_condattr_destroy(pthread_condattr_t* attr);
int pthread_condattr_getpshared(const pthread_condattr_t* attr, int* pshared);
int pthread_condattr_setpshared(pthread_condattr_t* attr, int pshared);

// RW Lock

int pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr);
int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);
int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t* rwlock);

int pthread_rwlockattr_init(pthread_rwlockattr_t* attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t* attr);

// Once
int pthread_once(pthread_once_t* once_control, void (*init_routine)(void));

// TLS, not supported!

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

#if defined(__cplusplus)
}
#endif
