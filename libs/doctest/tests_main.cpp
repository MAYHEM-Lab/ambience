//
// Created by Mehmet Fatih BAKIR on 13/04/2018.
//

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <tos/ft.hpp>

void tos_main() {
    tos::launch(tos::alloc_stack, [] {
        doctest::Context context;
        try {
            int res = context.run(); // run

            if (context.shouldExit()) {
                // important - query flags (and --exit) rely on the
                // user doing this
                // propagate the result of the tests
                exit(res);
                return res;
            }
            exit(res);
        }
        catch (std::exception& error) {
            std::cerr << error.what() << '\n';
            exit(1);
        }
    });
}