#include <tos/condition_variable.hpp>
#include <pthread.h>
#include <tos/shared_mutex.hpp>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>

// Anonymous namespace
namespace {

// Convert pthread_mutex_t to tos::mutex
tos::mutex& pmutex_to_tmutex(pthread_mutex_t* mutex) {
    return *std::launder(reinterpret_cast<tos::mutex*>(mutex->mutex_buffer));
}

// Convert pthread_cond_t to Condition
tos::condition_variable& pcond_to_tcond(pthread_cond_t* cond) {
    return *std::launder(reinterpret_cast<tos::condition_variable*>(cond->cond_buffer));
}

// Convert pthread_rwlock_t to RWLock
tos::shared_mutex& prwlock_to_trwlock(pthread_rwlock_t* rwlock) {
    return *std::launder(reinterpret_cast<tos::shared_mutex*>(rwlock->rwlock_buffer));
}

struct pthread_tcb : tos::kern::tcb {
    pthread_tcb(uint8_t* stack, void* (*entry)(void*), void* arg)
        : tcb(tos::current_context())
        , entry(entry)
        , arg(arg) {
        value_ptr = nullptr;
    }

    void start() {
        run_on_start();
        tos::kern::enable_interrupts();
        std::invoke(entry, arg);
        join_sem.up();
        exit_sem.down();
        tos::this_thread::exit(nullptr);
    }

    tos::semaphore join_sem{0};
    tos::semaphore exit_sem{0};
    void* (*entry)(void*);
    void* arg;
    void* value_ptr;
};

} // namespace

extern "C" {

/* Pthread */

int pthread_attr_init(pthread_attr_t* attr) {
    *attr = pthread_attr_t{};
    return 0;
}

int pthread_attr_destroy(pthread_attr_t* attr) {
    attr->stack_addr = nullptr;
    attr->stack_size = -1;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stack_size) {
    // Check parameters
    if (!attr)
        return 1;
    if (stack_size < TOS_DEFAULT_STACK_SIZE)
        return 1;

    // Set stack size
    attr->stack_size = stack_size;

    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t* attr, size_t* stack_size) {
    // Check parameters
    if (!attr || !stack_size)
        return 1;

    // Get stack size
    *stack_size = attr->stack_size;

    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t* attr, void* stack_addr) {
    // Check parameters
    if (!attr)
        return 1;

    // Set stack address
    attr->stack_addr = stack_addr;

    return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t* attr, void** stack_addr) {
    // Check parameters
    if (!attr || !stack_addr)
        return 1;

    // Get stack address
    *stack_addr = attr->stack_addr;

    return 0;
}

int pthread_create(pthread_t* thread,
                   const pthread_attr_t* attr,
                   void* (*start_routine)(void*),
                   void* arg) {

    size_t stack_size = 0;      // Holds requested size of stack (if specified)
    void* stack_addr = nullptr; // Holds requested stack address (if specified)

    // Get stack size and address from provided attributes
    if (attr) {
        auto stack_size_err = pthread_attr_getstacksize(attr, &stack_size);
        auto stack_addr_err = pthread_attr_getstackaddr(attr, &stack_addr);
        if (stack_size_err)
            return stack_size_err;
        if (stack_addr_err)
            return stack_addr_err;
    }

    // Allocate stack
    if (stack_size == 0) {
        stack_size = 4096;
    }
    if (!stack_addr) {
        stack_addr = new (std::nothrow) char[stack_size];
        if (!stack_addr) {
            tos::debug::panic("Stack allocation failed");
        }
    }
    auto& task = tos::raw_launch<pthread_tcb>(
        tos::span<uint8_t>((uint8_t*)stack_addr, stack_size), start_routine, arg);
    *thread = &task;

    return 0;
}

int pthread_join(pthread_t thread, void** value_ptr) {
    auto thread_str = static_cast<pthread_tcb*>(thread);
    thread_str->join_sem.down();
    if (value_ptr != nullptr) {
        *value_ptr = thread_str->value_ptr;
    }
    thread_str->exit_sem.up();
    return 0;
}

int pthread_detach(pthread_t thread) {
    auto thread_str = static_cast<pthread_tcb*>(thread);
    thread_str->exit_sem.up();
    return 0;
}
int pthread_equal(pthread_t t1, pthread_t t2) {
    if (t1 != t2) {
        return 1;
    }

    return 0;
}

void pthread_exit(void* value_ptr) {
    auto thread_str = static_cast<pthread_tcb*>(tos::self());
    thread_str->value_ptr = value_ptr;
    thread_str->join_sem.up();
    thread_str->exit_sem.down();
    tos::this_thread::exit(nullptr);
}

pthread_t pthread_self(void) {
    return tos::self();
}

/* Mutex */

int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
    static_assert(TOS_MUTEX_SIZE >= sizeof(tos::mutex), "TOS_MUTEX_SIZE is too small");
    new (mutex->mutex_buffer) tos::mutex;

    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) {
    std::destroy_at(&pmutex_to_tmutex(mutex));

    return 0;
}

int pthread_mutex_setprioceiling(pthread_mutex_t* mutex,
                                 int prioceiling,
                                 int* old_ceiling) {
    return 1;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t* mutex, int* prioceiling) {
    return 1;
}

int pthread_mutex_lock(pthread_mutex_t* mutex) {
    Assert(mutex != nullptr);
    pmutex_to_tmutex(mutex).lock();

    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t* mutex) {
    Assert(mutex != nullptr);
    pmutex_to_tmutex(mutex).try_lock();

    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    Assert(mutex != nullptr);
    pmutex_to_tmutex(mutex).unlock();

    return 0;
}


int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol) {
    // Check parameters
    if (!attr)
        return 1;

    // Set protocol
    attr->protocol = protocol;

    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t* attr, int* protocol) {
    // Check parameters
    if (!attr || !protocol)
        return 1;

    // Get protocol
    *protocol = attr->protocol;

    return 0;
}

int pthread_mutexattr_getpshared(const pthread_mutexattr_t* attr, int* pshared) {
    // Check parameters
    if (!attr || !pshared)
        return 1;

    // Get pshared
    *pshared = attr->pshared;

    return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared) {
    // Check parameters
    if (!attr)
        return 1;

    // Set pshared
    attr->pshared = pshared;

    return 0;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t* attr, int prioceiling) {
    return 1;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t* attr, int* prioceiling) {
    return 1;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t* attr, int* type) {
    // Check parameters
    if (!attr || !type)
        return 1;

    // Get type
    *type = attr->type;

    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type) {
    // Check parameters
    if (!attr)
        return 1;

    // Set type
    attr->type = type;

    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t* attr) {
    *attr = pthread_mutexattr_t{};
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t* attr) {
    return 0;
}

/* Condition Variable */

int pthread_cond_signal(pthread_cond_t* cond) {
    Assert(cond != nullptr);
    pcond_to_tcond(cond).notify_one();

    return 0;
}

int pthread_cond_broadcast(pthread_cond_t* cond) {
    Assert(cond != nullptr);
    pcond_to_tcond(cond).notify_all();

    return 0;
}

int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr) {
    static_assert(TOS_COND_SIZE >= sizeof(tos::condition_variable),
                  "TOS_COND_SIZE is too small");
    new (cond->cond_buffer) tos::condition_variable;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t* cond) {
    Assert(cond != nullptr);
    std::destroy_at(&pcond_to_tcond(cond));

    return 0;
}

int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    Assert(cond != nullptr);
    pcond_to_tcond(cond).wait(pmutex_to_tmutex(mutex));

    return 0;
}

int pthread_condattr_init(pthread_condattr_t* attr) {
    return 0;
}

int pthread_condattr_destroy(pthread_condattr_t* attr) {
    return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t* attr, int* pshared) {
    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t* attr, int pshared) {
    return 0;
}

/* RW Lock */

int pthread_rwlock_init(pthread_rwlock_t* rwlock, const pthread_rwlockattr_t* attr) {
    static_assert(TOS_RW_SIZE >= sizeof(tos::shared_mutex), "TOS_RW_SIZE is too small");
    new (rwlock->rwlock_buffer) tos::shared_mutex;
    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    std::destroy_at(&prwlock_to_trwlock(rwlock));

    return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    prwlock_to_trwlock(rwlock).lock_shared();

    return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    return !prwlock_to_trwlock(rwlock).try_lock_shared();
}

int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    prwlock_to_trwlock(rwlock).lock();

    return 0;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    return !prwlock_to_trwlock(rwlock).try_lock();
}

int pthread_rwlock_unlock(pthread_rwlock_t* rwlock) {
    Assert(rwlock != nullptr);
    prwlock_to_trwlock(rwlock).pthread_unlock();

    return 0;
}


int pthread_rwlockattr_init(pthread_rwlockattr_t* attr) {
    return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t* attr) {
    return 0;
}

} // extern C
