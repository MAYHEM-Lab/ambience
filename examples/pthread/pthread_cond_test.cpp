#include <arch/drivers.hpp>
#include <pthread.h>
#include <string>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <vector>

/* TESTING UTILITIES */

template<typename T, typename F>
bool run_test(std::string name, T expected, F test_func) {
    LOG("[ Running", name, "]");
    bool passed = (expected == test_func());

    std::string result = passed ? "PASSED ✓" : "FAILED ⨯";
    LOG("[", name, result, "]\n");

    return passed;
}

/* PRODUCER-CONSUMER TEST */

// Results
std::vector<std::string> expected_result;
std::vector<std::string> result;

// Declare synchronization primitives
pthread_mutex_t mutex;
pthread_cond_t cond;

bool condition = false;
int count = 0;
const int ITERATIONS = 10;

void* produce(void* arg) {
    while (count < ITERATIONS) {
        pthread_mutex_lock(&mutex);

        while (condition) {
            pthread_cond_wait(&cond, &mutex);
        }

        // Produce
        LOG("Produced: ", ++count);
        result.push_back("Produced: " + std::to_string(count));
        condition = true;

        // Wake consumer
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

void* consume(void* arg) {
    while (count < ITERATIONS) {
        pthread_mutex_lock(&mutex);

        while (!condition) {
            pthread_cond_wait(&cond, &mutex);
        }

        LOG("Consumed: ", count);
        result.push_back("Consumed: " + std::to_string(count));

        // Consume
        condition = false;

        // Wake producer
        pthread_cond_signal(&cond);

        // unlock the mutex
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

bool producer_consumer_test() {
    // Prepare expected result
    for (int i = 1; i <= ITERATIONS; i++) {
        expected_result.push_back("Produced: " + std::to_string(i));
        expected_result.push_back("Consumed: " + std::to_string(i));
    }

    // Declare threads
    pthread_t producer_thread;
    pthread_t consumer_thread;

    // Initialize synchronization primitives
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create threads
    pthread_create(&producer_thread, NULL, produce, NULL);
    pthread_create(&consumer_thread, NULL, consume, NULL);

    // Await thread completion
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    for (size_t i = 0; i < result.size(); i++)
        if (result[i] != expected_result[i])
            return false;

    return true;
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

    LOG("=== TESTING PTHREAD_COND (CONDITION VARIABLES) ===\n");

    run_test("Producer-Consumer Test", true, &producer_consumer_test);


    LOG("=== FINISHED TESTING PTHREAD_COND ===\n");

    return 0;
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_func);
}