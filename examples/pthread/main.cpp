#include <arch/drivers.hpp>
#include <pthread.h>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>

tos::semaphore sem1{0};
tos::semaphore sem2{1};

int i;

void another() {
    LOG("Hello world from thread 2!");
    while (true) {
        sem1.down();
        LOG("Tick!");
        sem2.up();
    }
}

void* test_mutex(void*) {
    // write a test that makes sure only one thread has the mutex at a time
    return nullptr;
}

void* hello_world(void*) {
    LOG("Hello world from thread", tos::this_thread::get_id().id);
    return nullptr;
}

void test_pthread_create() {
    pthread_t thread1;
    pthread_t thread2;
    auto result = pthread_create(&thread1, nullptr, &hello_world, nullptr);
    Assert(result == 0);
    // result = pthread_create(&thread2, nullptr, &hello_world, nullptr);
    // Assert(result == 0);
}

void test_condition_variables() {
}

void main_func() {
    tos::raspi3::interrupt_controller ic;
    [[maybe_unused]] auto uart =
        tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);

    // Set up logging
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    // Test pthreads
    LOG("TESTING PTHREADS");
    test_pthread_create();

    // Test condition variables
    LOG("TESTING CONDITION VARIABLES");
    test_condition_variables();

    tos::this_thread::block_forever();

    /*
    tos::raspi3::interrupt_controller ic;

    [[maybe_unused]] auto uart =
        tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    LOG("Hello world from thread 1!");

    pthread_attr_t attrs;
    pthread_t thread;

    pthread_create(&thread, &attrs, &print_hello_world, NULL);


    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, nullptr, &test_mutex, NULL);
    pthread_create(&thread2, nullptr, &test_mutex, NULL);
     *
     */


    //    pthread_mutex_t mut;
    //
    //    auto result = pthread_mutex_init(&mut, nullptr);
    //    Assert(result == 0 && "init did not work");
    //    result = pthread_mutex_lock(&mut);
    //    Assert(result == 0 && "lock returned nonzero");
    //    result = pthread_mutex_unlock(&mut);
    //    Assert(result == 0 && "unlock returned nonzero");

    //    tos::launch(tos::alloc_stack, another);
    //
    //    while (true) {
    //        sem2.down();
    //        LOG("Tock!");
    //        sem1.up();
    //    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_func);
}