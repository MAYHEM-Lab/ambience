#include <arch/drivers.hpp>
#include <pthread.h>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>

void* hello_world_no_exit(void* arg) {
    LOG("Hello world from thread", tos::this_thread::get_id().id);
    return nullptr;
}

void* hello_world_detached_thread(void* arg) {
    LOG("Hello world from DETACHED thread", tos::this_thread::get_id().id);
    return nullptr;
}

// Use arg as the value_ptr
void* hello_world_then_exit(void* arg) {
    LOG("Hello world from thread", tos::this_thread::get_id().id);
    pthread_exit(arg);
    return nullptr;
}

void* test_pthread_equal_helper(void* arg) {
    LOG("Test pthread equal helper", tos::this_thread::get_id().id);
    auto result = pthread_equal(pthread_self(), pthread_self());
    Assert(result == 0);
    pthread_t thread2;
    pthread_create(&thread2, nullptr, &hello_world_no_exit, arg);
    result = pthread_equal(pthread_self(), thread2);
    Assert(result != 0);
    pthread_join(thread2, nullptr);

    return nullptr;
}

void test_pthread_attr_init() {
    pthread_attr_t attr;
    auto result = pthread_attr_init(&attr);
    Assert(attr.stack_addr == nullptr);
    Assert(attr.stack_size == 0);
    Assert(result == 0);
    LOG("PASSED test_pthread_attr_init()");
}

void test_pthread_attr_destroy() {
    pthread_attr_t attr;
    auto result = pthread_attr_destroy(&attr);
    Assert(attr.stack_addr == nullptr);
    Assert(static_cast<int>(attr.stack_size) == -1);
    Assert(result == 0);
    LOG("PASSED test_pthread_attr_destroy()");
}

void test_pthread_attr_setstacksize() {
    pthread_attr_t attr;

    // Test stack size too small
    auto result = pthread_attr_setstacksize(&attr, TOS_DEFAULT_STACK_SIZE - 1);
    Assert(result != 0);

    // Test correct stack sizes
    result = pthread_attr_setstacksize(&attr, TOS_DEFAULT_STACK_SIZE);
    Assert(attr.stack_size == TOS_DEFAULT_STACK_SIZE);
    Assert(result == 0);
    result = pthread_attr_setstacksize(&attr, TOS_DEFAULT_STACK_SIZE + 1024);
    Assert(attr.stack_size == TOS_DEFAULT_STACK_SIZE + 1024);
    Assert(result == 0);

    LOG("PASSED test_pthread_attr_setstacksize()");
}

// This test is dependent on pthread_attr_setstacksize working
void test_pthread_attr_getstacksize() {
    pthread_attr_t attr;
    pthread_attr_setstacksize(&attr, TOS_DEFAULT_STACK_SIZE);
    size_t stacksize;
    auto result = pthread_attr_getstacksize(&attr, &stacksize);
    Assert(result == 0);
    Assert(stacksize == TOS_DEFAULT_STACK_SIZE);
    LOG("PASSED test_pthread_attr_getstacksize()");
}

void test_pthread_attr_setstackaddr() {
    pthread_attr_t attr;
    void* stack_addr = &attr + 1024; // dummy address
    auto result = pthread_attr_setstackaddr(&attr, stack_addr);
    Assert(result == 0);
    Assert(attr.stack_addr == stack_addr);
    LOG("PASSED test_pthread_attr_setstackaddr()");
}

// This test is dependent on pthread_attr_setstackaddr working
void test_pthread_attr_getstackaddr() {
    pthread_attr_t attr;
    void* stack_addr_test = &attr + 1024; // dummy address
    pthread_attr_setstackaddr(&attr, stack_addr_test);
    void* stack_addr_real;
    auto result = pthread_attr_getstackaddr(&attr, &stack_addr_real);
    Assert(result == 0);
    Assert(stack_addr_real == stack_addr_test);
    LOG("PASSED test_pthread_attr_getstackaddr()");
}

void test_pthread_create_join() {
    LOG("RUNNING test_pthread_create_join()");
    pthread_t thread;
    auto result = pthread_create(&thread, nullptr, &hello_world_no_exit, nullptr);
    Assert(result == 0);
    void* value_ptr;
    result = pthread_join(thread, &value_ptr);
    Assert(result == 0);
    LOG("PASSED test_pthread_create_join()");
}

void test_pthread_create_exit_join() {
    LOG("RUNNING test_pthread_create_exit_join()");
    pthread_t thread;
    void* arg = &thread;
    auto result = pthread_create(&thread, nullptr, &hello_world_then_exit, arg);
    Assert(result == 0);
    void* value_ptr;
    result = pthread_join(thread, &value_ptr);
    Assert(result == 0);
    Assert(value_ptr == arg);
    LOG("PASSED test_pthread_create_exit_join()");
}

void test_pthread_create_twice() {
    LOG("RUNNING test_pthread_create_twice()");
    pthread_t thread1;
    void* arg = &thread1;
    auto result = pthread_create(&thread1, nullptr, &hello_world_then_exit, arg);
    Assert(result == 0);
    pthread_t thread2;
    result = pthread_create(&thread2, nullptr, &hello_world_then_exit, arg);
    Assert(result == 0);
    void* value_ptr1;
    result = pthread_join(thread1, &value_ptr1);
    Assert(result == 0);
    void* value_ptr2;
    result = pthread_join(thread2, &value_ptr2);
    Assert(result == 0);
    LOG("PASSED test_pthread_create_twice()");
}

void test_pthread_detach() {
    LOG("RUNNING test_pthread_detach()");
    pthread_t thread;
    pthread_create(&thread, nullptr, &hello_world_detached_thread, nullptr);
    auto result = pthread_detach(thread);
    Assert(result == 0);
    LOG("PASSED test_pthread_detach()");
}

void test_pthread_equal() {
    LOG("RUNNING test_pthread_equal()");
    pthread_t thread;
    pthread_create(&thread, nullptr, &test_pthread_equal_helper, nullptr);
    pthread_join(thread, nullptr);
    LOG("PASSED test_pthread_equal");
}

void main_func() {
    // Set up
    tos::raspi3::interrupt_controller ic;
    [[maybe_unused]] auto uart =
        tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    LOG("STARTING PTHREAD TESTS");
    test_pthread_attr_init();
    test_pthread_attr_destroy();
    test_pthread_attr_setstacksize();
    test_pthread_attr_getstacksize();
    test_pthread_attr_setstackaddr();
    test_pthread_attr_getstackaddr();
    test_pthread_create_join();
    test_pthread_create_exit_join();
    test_pthread_create_twice();
    test_pthread_detach();
    test_pthread_equal();

    // Not sure why we this is necessary...
    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_func);
}
