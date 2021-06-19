#include <arch/drivers.hpp>
#include <pthread.h>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>

// Test: Get read lock when another reader has it simultaneously (should acquire lock)

pthread_rwlock_t rwlock1;
bool accessed1 = false;

void* test_rdlock_1_func(void* arg) {
    Assert(pthread_rwlock_rdlock(&rwlock1) == 0);
    accessed1 = true;
    Assert(pthread_rwlock_unlock(&rwlock1) == 0);
    return nullptr;
}

void test_rdlock_1() {
    LOG("RUNNING test_rdlock1");
    pthread_t t;
    pthread_rwlock_init(&rwlock1, nullptr);

    Assert(pthread_rwlock_rdlock(&rwlock1) == 0);
    Assert(pthread_create(&t, nullptr, &test_rdlock_1_func, nullptr) == 0);
    Assert(pthread_join(t, NULL) == 0);
    Assert(pthread_rwlock_unlock(&rwlock1) == 0);

    Assert(accessed1);

    LOG("PASSED: test_rdlock_1()\n");
}

// Test: Get read lock when another reader has it simultaneously (should acquire lock)

pthread_rwlock_t rwlock2;
bool accessed2 = false;

void* test_tryrdlock_1_func(void* arg) {
    Assert(pthread_rwlock_tryrdlock(&rwlock2) == 0);
    Assert(pthread_rwlock_unlock(&rwlock2) == 0);
    accessed2 = true;
    return nullptr;
}

void test_tryrdlock_1() {
    LOG("RUNNING test_tryrdlock_1");
    pthread_t t;
    pthread_rwlock_init(&rwlock2, nullptr);

    Assert(pthread_rwlock_rdlock(&rwlock2) == 0);
    Assert(pthread_create(&t, nullptr, &test_tryrdlock_1_func, nullptr) == 0);
    Assert(pthread_join(t, NULL) == 0);
    Assert(pthread_rwlock_unlock(&rwlock2) == 0);

    Assert(accessed2);

    LOG("PASSED: test_tryrdlock_1()\n");
}

// Test: Attempt to get write lock when there is already a reader (should not acquire
// lock)

pthread_rwlock_t rwlock3;
bool accessed3 = false;

void* test_trywrlock_1_func(void* arg) {
    if (!pthread_rwlock_trywrlock(&rwlock3)) {
        LOG("GOT LE LOCK");
        accessed3 = true;
        pthread_rwlock_unlock(&rwlock3);
    }

    return nullptr;
}

void test_trywrlock_1() {
    LOG("RUNNING test_trywrlock_1");
    pthread_t t;
    pthread_rwlock_init(&rwlock3, nullptr);

    Assert(pthread_rwlock_rdlock(&rwlock3) == 0);
    Assert(pthread_create(&t, nullptr, &test_trywrlock_1_func, nullptr) == 0);
    Assert(pthread_join(t, NULL) == 0);
    Assert(pthread_rwlock_unlock(&rwlock3) == 0);

    Assert(!accessed3);

    LOG("PASSED: test_trywrlock_1()");
}

/* RUN ALL TESTS */

int main_func() {
    /* Setup */
    tos::raspi3::interrupt_controller ic;
    [[maybe_unused]] auto uart =
        tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);
    /* Setup */

    LOG("=== TESTING PTHREAD_RWLOCK (SHARED MUTEX) ===\n");

    test_rdlock_1();
    test_tryrdlock_1();
    test_trywrlock_1();

    LOG("=== FINISHED TESTING PTHREAD_RWLOCK ===\n");

    return 0;
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_func);
}