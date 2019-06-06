//
// Created by Mehmet Fatih BAKIR on 13/04/2018.
//

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include <tos/ft.hpp>

void tos_main()
{
    tos::launch(tos::alloc_stack, []{
        doctest::Context context;
        int res = context.run(); // run

        if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
            return res;          // propagate the result of the tests

        return 0;
    });
}