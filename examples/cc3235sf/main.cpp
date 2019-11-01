//
// Created by fatih on 10/31/19.
//

volatile int foo;
void tos_main() {
    while (true) {
        ++foo;
        asm volatile("BKPT 0");
    }
}