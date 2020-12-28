#include <tos/ft.hpp>

void thread() {
    while (true) {
        tos::this_thread::yield();
    }
}

tos::stack_storage store;
void tos_main() {
    tos::launch(store, thread);
}